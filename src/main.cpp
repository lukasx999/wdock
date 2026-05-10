#include <cstdlib>
#include <format>
#include <thread>

#include "application.hpp"
#include "window.hpp"
#include "config.hpp"
#include "config_watcher.hpp"

int main() {

    auto config_path = "config.kdl";

    std::optional<application> app;

    try {
        app.emplace();
    } catch (const window_error& error) {
        print_error("failed to open window: {}", error.what());
        return EXIT_FAILURE;
    }

    config_watcher watcher(*app, config_path);

    try {
        app->load_config(config_path);
        print_info("config loaded from \"{}\"", config_path);
        app->run();

    } catch (const config_error& error) {
        print_error("failed to load config file: {}", error.what());
        watcher.stop();
        return EXIT_FAILURE;

    } catch (const widget_error& error) {
        print_error("failed to configure widget: {}", error.what());
        watcher.stop();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}