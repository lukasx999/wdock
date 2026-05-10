#pragma once

#include <filesystem>
#include <thread>

#include <sys/inotify.h>
#include <poll.h>

#include "application.hpp"
#include "utils.hpp"

class config_watcher {
    public:
    config_watcher(application& app, const std::filesystem::path& path)
    {

        m_thread = std::jthread([&](std::stop_token stop_token) {
            auto fn = std::bind(reload_config, std::ref(app), path);
            auto stop_fn = std::bind(&std::stop_token::stop_requested, stop_token);

            if (not watch_file(path, fn, stop_fn))
                print_error("failed to install watcher for config file at \"{}\"", path.string());
        });

    }

    private:
    std::jthread m_thread;

    static void reload_config(application& app, const std::filesystem::path& path) {
        std::scoped_lock lock(g_draw_lock);

        try {
            app.load_config(path);
            print_info("config was reloaded from \"{}\"", path.string());
        } catch (const config_error& error) {
            print_error("failed to reload config: {}", error.what());
        }

    }

    /// @brief calls a function whenever a file is modified.
    /// @param fn the function to be called when the file is modified
    /// @param stop_fn stops the watching process and returns if this function returns true
    /// @return returns false if the watcher could not be installed
    static bool watch_file(const std::filesystem::path& path, std::function<void()> fn, std::function<bool()> stop_fn) {

        // vim will only reliably produce IN_MOVE_SELF events, so we have to catch those
        auto inotify_flags = IN_MODIFY | IN_CLOSE_WRITE | IN_MOVE_SELF;

        int fd = inotify_init1(IN_NONBLOCK);
        if (fd == -1)
            return false;

        int wd = inotify_add_watch(fd, path.c_str(), inotify_flags);
        if (wd == -1)
            return false;

        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;

        while (!stop_fn()) {
            int num_fds = poll(&pfd, 1, 100);
            if (num_fds == -1)
                return false;

            if (num_fds == 1) {
                if (!(pfd.revents & POLLIN)) continue;

                if (not handle_inotify_event(fd, wd, path, inotify_flags, fn))
                    return false;
            }

        }

        if (inotify_rm_watch(fd, wd) == -1)
            return false;

        if (close(fd) == -1)
            return false;

        return true;
    }

    static bool handle_inotify_event(int fd, int wd, const std::filesystem::path& path, int flags, std::function<void()> fn) {

        struct inotify_event event;
        ssize_t bytes_read = read(fd, &event, sizeof event);
        if (bytes_read == 0 || bytes_read == -1)
            return false;

        // vim will actually swap the edited file with a new file, so
        // we have to catch that and add the file back to the watchlist
        if (event.mask & IN_IGNORED) {
            while (true) {
                wd = inotify_add_watch(fd, path.c_str(), flags);
                if (wd != -1) break;
                if (errno != ENOENT)
                    return false;
            }
        }

        fn();

        return true;
    }

};