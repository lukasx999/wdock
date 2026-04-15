#include <cstdlib>
#include <iostream>
#include <fstream>
#include <format>
#include <print>

#include "application.hpp"
#include "widgets.hpp"
#include "window.hpp"

#include <kdl/kdl.h>
#include <kdlpp.h>

struct config {
    struct window {
        std::u8string font;
        float fontsize;
        std::u8string background_color;
        float border_radius;
        float padding ;
        int width;
        int height;
        ::window::anchor anchor;
        ::window::margin margin;
    } window;

    struct widget_definition {
        std::u8string preset;
        std::unordered_map<std::u8string, std::vector<kdl::Value>> properties;
    };

    std::unordered_map<std::u8string, widget_definition> widget_definitions;
    std::vector<std::u8string> used_widgets;

};

[[nodiscard]] struct config::window parse_window_config(kdl::Node& node) {
    struct config::window window;

    for (auto& child : node.children()) {
        auto& name = child.name();

        if (name == u8"font") {
            window.font = child.args()[0].as<std::u8string>();

        } else if (name == u8"fontsize") {
            window.fontsize = child.args()[0].as<float>();

        } else if (name == u8"width") {
            window.width = child.args()[0].as<kdl::Number>().as<int>();

        } else if (name == u8"height") {
            window.height = child.args()[0].as<kdl::Number>().as<int>();

        } else if (name == u8"anchor") {
            auto anchor_str = child.args()[0].as<std::u8string>();

            window.anchor = [&] {
                if (anchor_str == u8"top")
                    return window::anchor::top;
                if (anchor_str == u8"right")
                    return window::anchor::right;
                if (anchor_str == u8"bottom")
                    return window::anchor::bottom;
                if (anchor_str == u8"left")
                    return window::anchor::left;
                else
                    assert(!"invalid anchor value");

            }();

        }

    }

    return window;
}

[[nodiscard]] config parse_config_file(const std::filesystem::path& config_path) {
    std::ifstream stream(config_path);
    std::u8string config_src(std::istreambuf_iterator<char>(stream), {});
    auto doc = kdl::parse(config_src);

    config config;
    for (auto& node : doc.nodes()) {
        if (node.name() == u8"window") {
            config.window =  parse_window_config(node);

        } else if (node.name() == u8"widgets") {
            for (auto& child : node.children()) {
                config.used_widgets.push_back(child.name());
            }

        } else if (node.name() == u8"widget") {
            std::u8string name = node.args()[0].as<std::u8string>();
            std::u8string preset = node.properties()[u8"preset"].as<std::u8string>();
            std::unordered_map<std::u8string, std::vector<kdl::Value>> properties;

            for (auto& child : node.children()) {
                properties[child.name()] = child.args();
            }

            config.widget_definitions[name] = { preset, properties };
        }

    }

    return config;
}

int main() {

    const char* config_path = "config.kdl";
    config config = parse_config_file(config_path);

    int width = 700;
    int height = 800;
    auto anchor = window::anchor::right;
    window::margin margin = {0, 200, 0, 0};

    try {
        application app(width, height, anchor, margin);

        for (auto& widget : config.used_widgets) {
            auto def = config.widget_definitions[widget];

            if (def.preset == u8"datetime") {
                app.add_widget<widgets::datetime>("Europe/Vienna", " %d.%m.%Y");
            }

        }

        // widgets must be added AFTER the application has been constructed, as this
        // is when the opengl context gets initialized, which a widget might use
        app.add_widget<widgets::datetime>("Europe/Vienna", " %d.%m.%Y");
        app.add_widget<widgets::datetime>("Europe/Vienna", " %H:%M");
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