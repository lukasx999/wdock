#include <cstdlib>
#include <iostream>
#include <functional>
#include <format>
#include <print>
#include <thread>

#include "application.hpp"
#include "widgets.hpp"
#include "window.hpp"
#include "config.hpp"

#include <sys/inotify.h>

int main2() {
    window a("a", 500, 500);
    window b("b", 500, 500);

    a.set_anchor(window::anchor::left);
    b.set_anchor(window::anchor::right);

    a.on_draw([] {
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    });

    b.on_draw([] {
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    });

    std::jthread ta([&] {
        a.run();
    });

    std::jthread tb([&] {
        b.run();
    });

    // std::vector<const window*> windows = { &a, &b };
    // window::run_concurrent(windows);

    return 0;
}

void watch_file(const std::filesystem::path& path, std::invocable auto fn) {

    int fd = inotify_init();
    assert(fd != -1);

    auto flags = IN_MODIFY;

    int wd = inotify_add_watch(fd, path.c_str(), flags);
    assert(wd != -1);

    while (true) {
        struct inotify_event event;
        ssize_t bytes_read = read(fd, &event, sizeof event);
        assert(bytes_read != -1);
        assert(bytes_read != 0);

        if (event.mask & IN_IGNORED) {
            wd = -1;
            while (wd == -1)
                wd = inotify_add_watch(fd, path.c_str(), flags);
        }

        fn();
    }

    assert(inotify_rm_watch(fd, wd) != -1);
    assert(close(fd) != -1);
}

int main() {

    // watch_file("/home/lukas/code/repos/wdock/foo.txt", [&] {
    // });

    auto config_path = "config.kdl";

    try {
        application app;

        std::jthread watcher_thread([&] {
            watch_file(config_path, [&] {
                std::scoped_lock lock(mutex);
                config config;

                try {
                    config = parse_config(config_path);
                } catch (const config_error& error) {
                    std::println(std::cerr, "failed to reload config: {}", error.what());
                    return;
                }

                app.load_config(config);
                std::println("config was reloaded");
            });
        });

        // parse_config() must be called AFTER the application has been constructed, because it
        // may call widget constructors, which may call into OpenGL functions.
        config config = parse_config(config_path);
        app.load_config(config);
        app.run();

    } catch (const config_error& error) {
        std::println(std::cerr, "failed to parse config file: {}", error.what());
        return EXIT_FAILURE;

    } catch (const window_error& error) {
        std::println(std::cerr, "failed to open window: {}", error.what());
        return EXIT_FAILURE;

    } catch (const widget_error& error) {
        std::println(std::cerr, "failed to add widget: {}", error.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}