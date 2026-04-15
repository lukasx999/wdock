#include <cstdlib>
#include <iostream>
#include <format>
#include <print>

#include "application.hpp"
#include "widgets.hpp"
#include "window.hpp"
#include "config.hpp"

int main() {

    const char* config_path = "config.kdl";
    config config = parse_config_file(config_path);

    auto window = config.window;
    int width   = window.width;
    int height  = window.height;
    auto anchor = window.anchor;
    auto margin = window.margin;

    try {
        application app(width, height, anchor, margin);

        for (auto& widget : config.used_widgets) {
            auto def = config.widget_definitions[widget];

            if (def.preset == "datetime") {
                auto timezone = string_from_u8string(def.properties["timezone"].front().as<std::u8string>());
                auto format = string_from_u8string(def.properties["format"].front().as<std::u8string>());
                app.add_widget<widgets::datetime>(timezone, format);

            } else if (def.preset == "image") {
                auto path = string_from_u8string(def.properties["path"].front().as<std::u8string>());
                auto scaling = def.properties["scaling"].front().as<float>();
                app.add_widget<widgets::image>(path, scaling);

            }

        }

        // widgets must be added AFTER the application has been constructed, as this
        // is when the opengl context gets initialized, which a widget might use
        // app.add_widget<widgets::datetime>("Europe/Vienna", " %d.%m.%Y");
        // app.add_widget<widgets::datetime>("Europe/Vienna", " %H:%M");
        app.add_widget<widgets::kernel>();
        app.add_widget<widgets::memory>();
        app.add_widget<widgets::image>("image.png", 1);

        app.run();

    } catch (const window_error& error) {
        std::println(std::cerr, "failed to open window: {}", error.what());
        return EXIT_FAILURE;

    } catch (const widgets::widget_error& error) {
        std::println(std::cerr, "failed to add widget: {}", error.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}