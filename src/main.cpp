#include <cstdlib>
#include <iostream>
#include <format>
#include <print>

#include "application.hpp"
#include "widgets.hpp"
#include "window.hpp"
#include "config.hpp"

namespace {

    void add_widgets(application& app, const config& config) {
        for (auto& widget_name : config.widgets) {
            if (!config.widget_definitions.contains(widget_name))
                throw std::runtime_error(std::format("widget \"{}\" does not exist.", widget_name));

            auto widget_def = config.widget_definitions.at(widget_name);

            if (widget_def.preset == "datetime") {
                // TODO: add checking for invalid options
                auto timezone = string_from_u8(widget_def.properties["timezone"].front().as<std::u8string>());
                auto format = string_from_u8(widget_def.properties["format"].front().as<std::u8string>());

                app.add_widget<widgets::datetime>(timezone, format);

            } else if (widget_def.preset == "image") {
                auto path = string_from_u8(widget_def.properties["path"].front().as<std::u8string>());
                auto scaling = widget_def.properties["scaling"].front().as<float>();
                app.add_widget<widgets::image>(path, scaling);

            } else if (widget_def.preset == "kernel") {
                app.add_widget<widgets::kernel>();

            } else if (widget_def.preset == "button") {
                auto label = string_from_u8(widget_def.properties["label"].front().as<std::u8string>());
                app.add_widget<widgets::button>(label);

            } else
                throw std::runtime_error(std::format("widget preset \"{}\" does not exist.", widget_def.preset));

        }

    }

} // namespace

int main() {

    const char* config_path = "config.kdl";

    config config;
    try {
        config = parse_config_file(config_path);

    } catch (const config_error& error) {
        std::println(std::cerr, "failed to parse config file: {}", error.what());
        return EXIT_FAILURE;
    }

    auto window = config.window;
    int width   = window.width;
    int height  = window.height;
    auto anchor = window.anchor;
    auto margin = window.margin;

    try {
        application app(width, height, anchor, margin);

        // widgets must be added AFTER the application has been constructed, as this
        // is when the opengl context gets initialized, which a widget might use
        add_widgets(app, config);

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