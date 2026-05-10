#include <cstdlib>
#include <format>
#include <thread>

#include "application.hpp"
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

    std::jthread config_watcher([&](std::stop_token stop_token) {
        std::function<bool()> stop_fn = std::bind(&std::stop_token::stop_requested, stop_token);
        if (not watch_file_async(config_path, std::bind(reload_config, std::ref(*app), config_path), stop_fn))
            print_error("failed to install watcher for config file at \"{}\"", config_path);
    });

    try {
        app->load_config(config_path);
        print_info("config loaded from \"{}\"", config_path);
        app->run();

    } catch (const config_error& error) {
        print_error("failed to load config file: {}", error.what());
        config_watcher.request_stop();
        return EXIT_FAILURE;

    } catch (const widget_error& error) {
        print_error("failed to configure widget: {}", error.what());
        config_watcher.request_stop();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}