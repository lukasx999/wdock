#include "application.hpp"
#include "config.hpp"
#include "config_watcher.hpp"
#include "print.hpp"

int main() {

    auto config_path = "config.kdl";

    try {
        application app;
        config_watcher watcher(app, config_path);

        app.load_config(config_path);
        print_info("config loaded from \"{}\"", config_path);
        app.run();

    } catch (const config_error& error) {
        print_error("failed to load config file: {}", error.what());
        return EXIT_FAILURE;

    } catch (const widget_error& error) {
        print_error("failed to configure widget: {}", error.what());
        return EXIT_FAILURE;

    } catch (const window_error& error) {
        print_error("failed to open window: {}", error.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}