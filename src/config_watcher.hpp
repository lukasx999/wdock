#pragma once

#include <filesystem>
#include <thread>

#include <sys/inotify.h>
#include <poll.h>

#include "application.hpp"

class config_watcher {
    public:
    config_watcher(application& app, std::filesystem::path path);

    private:
    std::jthread m_thread;
    application& m_app;
    const std::filesystem::path m_config_path;
    const int m_poll_timeout_ms = 50;

    // vim will only reliably produce IN_MOVE_SELF events, so we have to catch those
    const int m_inotify_flags = IN_MODIFY | IN_CLOSE_WRITE | IN_MOVE_SELF;

    void reload_config() const;
    bool watch_file(std::function<bool()> stop_fn) const;
    bool handle_inotify_event(int fd, int wd) const;

};