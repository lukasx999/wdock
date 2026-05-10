#include "config_watcher.hpp"
#include "print.hpp"

config_watcher::config_watcher(application& app, std::filesystem::path path)
: m_app(app)
, m_config_path(std::move(path))
{

    m_thread = std::jthread([&](std::stop_token stop_token) {
        auto stop_fn = std::bind(&std::stop_token::stop_requested, stop_token);

        if (not watch_file(stop_fn))
            print_warning("failed to install watcher for config file at \"{}\"", path.string());
    });

}

void config_watcher::reload_config() const {
    std::scoped_lock lock(g_draw_lock);

    try {
        m_app.load_config(m_config_path);
        print_info("config was reloaded from \"{}\"", m_config_path.string());
    } catch (const config_error& error) {
        print_error("failed to reload config: {}", error.what());
    }

}

bool config_watcher::watch_file(std::function<bool()> stop_fn) const {

    int fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1)
        return false;

    int wd = inotify_add_watch(fd, m_config_path.c_str(), m_inotify_flags);
    if (wd == -1)
        return false;

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    while (!stop_fn()) {
        int num_fds = poll(&pfd, 1, m_poll_timeout_ms);
        if (num_fds == -1)
            return false;

        if (num_fds == 1) {
            if (!(pfd.revents & POLLIN)) continue;

            if (not handle_inotify_event(fd, wd))
                return false;
        }

    }

    if (inotify_rm_watch(fd, wd) == -1)
        return false;

    if (close(fd) == -1)
        return false;

    return true;
}

bool config_watcher::handle_inotify_event(int fd, int wd) const {

    struct inotify_event event;
    ssize_t bytes_read = read(fd, &event, sizeof event);
    if (bytes_read == 0 || bytes_read == -1)
        return false;

    // vim will actually swap the edited file with a new file, so
    // we have to catch that and add the new file back to the watchlist.
    if (event.mask & IN_IGNORED) {
        while (true) {
            wd = inotify_add_watch(fd, m_config_path.c_str(), m_inotify_flags);
            if (wd != -1) break;
            if (errno != ENOENT)
                return false;
        }
    }

    reload_config();

    return true;
}