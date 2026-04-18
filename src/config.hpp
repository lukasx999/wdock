#pragma once

#include <filesystem>

#include <kdl/kdl.h>
#include <kdlpp.h>

#include "window.hpp"
#include "widgets.hpp"

struct config_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct config {
    struct window {
        struct size {
            int width, height;
        };
        std::optional<size> size;
        std::optional<::window::anchor> anchor;
        std::optional<::window::layer> layer;
        std::optional<::window::margin> margin;

        // TODO: this should be a struct window_style
        std::string font;
        float fontsize;
        std::string background_color;
        float border_radius;
        float padding;
    };

    struct widget_definition {
        using properties = std::unordered_map<std::string, std::vector<kdl::Value>>;
        std::string preset;
        properties props;
    };

    window window;
    std::vector<std::string> widgets;
    std::unordered_map<std::string, widget_definition> widget_definitions;

    // TODO:
    // std::vector<std::unique_ptr<widgets::widget>> used_widgets;

};

[[nodiscard]] inline std::string string_from_u8(std::u8string str) {
    return { str.begin(), str.end() };
}

[[nodiscard]] config parse_config(const std::filesystem::path& config_path);