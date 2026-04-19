#include <cstdlib>
#include <iostream>
#include <format>
#include <print>

#include "application.hpp"
#include "widgets.hpp"
#include "window.hpp"
#include "config.hpp"

int main() {

    auto config_path = "config.kdl";

    try {
        application app(100, 100);

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