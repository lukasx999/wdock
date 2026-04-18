#pragma once

#include <fstream>
#include <filesystem>
#include <ranges>

#include <kdl/kdl.h>
#include <kdlpp.h>

#include "window.hpp"

struct config_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct config {
    struct window {
        std::string font;
        float fontsize;
        std::string background_color;
        float border_radius;
        float padding ;
        int width;
        int height;
        ::window::anchor anchor;
        ::window::margin margin;
    };

    struct widget_definition {
        std::string preset;
        std::unordered_map<std::string, std::vector<kdl::Value>> properties;
    };

    window window;
    std::vector<std::string> widgets;
    std::unordered_map<std::string, widget_definition> widget_definitions;

};

[[nodiscard]] inline std::string string_from_u8(std::u8string str) {
    return { str.begin(), str.end() };
}

// TODO: handle type errors
[[nodiscard]] inline struct config::window parse_window_config(kdl::Node& node) {

    struct config::window window;

    for (auto& child : node.children()) {
        auto name = string_from_u8(child.name());
        auto& args = child.args();

        if (name == "font") {
            window.font = string_from_u8(args[0].as<std::u8string>());

        } else if (name == "fontsize") {
            window.fontsize = args[0].as<float>();

        } else if (name == "background-color") {
            window.background_color = string_from_u8(args[0].as<std::u8string>());

        } else if (name == "border-radius") {
            window.border_radius = args[0].as<float>();

        } else if (name == "padding") {
            window.padding = args[0].as<float>();

        } else if (name == "width") {
            window.width = args[0].as<int>();

        } else if (name == "height") {
            window.height = args[0].as<int>();

        } else if (name == "anchor") {
            auto anchor_str = args[0].as<std::u8string>();

            window.anchor = [&] {
                if (anchor_str == u8"top")    return window::anchor::top;
                if (anchor_str == u8"right")  return window::anchor::right;
                if (anchor_str == u8"bottom") return window::anchor::bottom;
                if (anchor_str == u8"left")   return window::anchor::left;
                else throw config_error("invalid anchor value");
            }();

        } else if (name == "margin") {
            auto& props = child.properties();
            window.margin.top    = props[u8"top"].as<int>();
            window.margin.right  = props[u8"right"].as<int>();
            window.margin.bottom = props[u8"bottom"].as<int>();
            window.margin.left   = props[u8"left"].as<int>();

        } else
            throw config_error(std::format("invalid window option \"{}\"", name));

    }

    return window;
}

inline void parse_widget_definition(kdl::Node& node, config& config) {
    std::string name = string_from_u8(node.args()[0].as<std::u8string>());

    if (config.widget_definitions.contains(name))
        throw config_error(std::format("widget named \"{}\" has been defined multiple times", name));

    auto& def = config.widget_definitions[name];
    def.preset =  string_from_u8(node.properties()[u8"preset"].as<std::u8string>());

    for (auto& child : node.children()) {
        def.properties[string_from_u8(child.name())] = child.args();
    }

}

[[nodiscard]] inline config parse_config_file(const std::filesystem::path& config_path) {
    std::ifstream stream(config_path);
    std::u8string config_src(std::istreambuf_iterator<char>(stream), {});
    auto doc = kdl::parse(config_src);

    config config;
    for (auto& node : doc.nodes()) {
        auto name = string_from_u8(node.name());

        if (name == "window") {
            config.window = parse_window_config(node);

        } else if (name == "widgets") {
            config.widgets = node.args()
                | std::views::transform(&kdl::Value::as<std::u8string>)
                | std::ranges::to<std::vector<std::string>>();

        } else if (name == "define-widget") {
            parse_widget_definition(node, config);

        } else
            throw config_error(std::format("invalid config option: \"{}\"", name));

    }

    return config;
}