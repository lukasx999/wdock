#include <cstdlib>
#include <iostream>
#include <format>
#include <print>
#include <thread>
#include <cstdio>

#include "application.hpp"
#include "widgets.hpp"
#include "window.hpp"
#include "config.hpp"

namespace {

    void reload_config(application& app, const std::filesystem::path& config_path) {
        std::scoped_lock lock(g_draw_lock);

        try {
            app.load_config(config_path);
            print_info("config was reloaded from \"{}\"", config_path.string());
        } catch (const config_error& error) {
            print_error("failed to reload config: {}", error.what());
        }

    }

} // namespace

int main() {

    auto config_path = "config.kdl";

    std::optional<application> app;

    try {
        app.emplace();
    } catch (const window_error& error) {
        print_error("failed to open window: {}", error.what());
        return EXIT_FAILURE;
    }

    // TODO: stop this thread if an exception is thrown
    std::jthread config_watcher([&] {
        watch_file(config_path, std::bind(reload_config, std::ref(*app), config_path));
    });

    try {
        app->load_config(config_path);
        print_info("config loaded from \"{}\"", config_path);
        app->run();

    } catch (const config_error& error) {
        print_error("failed to load config file: {}", error.what());
        return EXIT_FAILURE;

    } catch (const widget_error& error) {
        print_error("failed to configure widget: {}", error.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}