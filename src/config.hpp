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

// TODO: handle type errors

[[nodiscard]] inline struct config::window parse_window_config(kdl::Node& node) {
    struct config::window window;

    for (auto& child : node.children()) {
        auto& name = child.name();
        auto& args = child.args();

        if (name == u8"font") {
            window.font = args[0].as<std::u8string>();

        } else if (name == u8"fontsize") {
            window.fontsize = args[0].as<float>();

        } else if (name == u8"background-color") {
            window.background_color = args[0].as<std::u8string>();

        } else if (name == u8"border-radius") {
            window.border_radius = args[0].as<float>();

        } else if (name == u8"padding") {
            window.padding = args[0].as<float>();

        } else if (name == u8"width") {
            window.width = args[0].as<int>();

        } else if (name == u8"height") {
            window.height = args[0].as<int>();

        } else if (name == u8"anchor") {
            auto anchor_str = args[0].as<std::u8string>();

            window.anchor = [&] {
                if (anchor_str == u8"top")    return window::anchor::top;
                if (anchor_str == u8"right")  return window::anchor::right;
                if (anchor_str == u8"bottom") return window::anchor::bottom;
                if (anchor_str == u8"left")   return window::anchor::left;
                else throw config_error("invalid anchor value");
            }();

        } else if (name == u8"margin") {
            auto& props = child.properties();
            window.margin.top    = props[u8"top"].as<int>();
            window.margin.right  = props[u8"right"].as<int>();
            window.margin.bottom = props[u8"bottom"].as<int>();
            window.margin.left   = props[u8"left"].as<int>();
        }

    }

    return window;
}

[[nodiscard]] inline config parse_config_file(const std::filesystem::path& config_path) {
    std::ifstream stream(config_path);
    std::u8string config_src(std::istreambuf_iterator<char>(stream), {});
    auto doc = kdl::parse(config_src);

    config config;
    for (auto& node : doc.nodes()) {
        auto& name = node.name();

        if (name == u8"window") {
            config.window = parse_window_config(node);

        } else if (name == u8"widgets") {
            config.used_widgets = node.args()
                | std::views::transform(&kdl::Value::as<std::u8string>)
                | std::ranges::to<std::vector<std::u8string>>();

        } else if (name == u8"widget") {
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