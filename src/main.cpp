#include <cstdlib>
#include <iostream>
#include <format>
#include <print>
#include <thread>

#include "application.hpp"
#include "widgets.hpp"
#include "window.hpp"
#include "config.hpp"

int main() {

    auto config_path = "config.kdl";

    try {
        application app;

        std::jthread config_watcher([&] {
            watch_file(config_path, [&] {
                std::scoped_lock lock(g_mutex);
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