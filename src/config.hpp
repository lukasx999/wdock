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
    private:
    template <typename T>
    using opt = std::optional<T>;

    public:
    struct window {
        struct size {
            int width, height;
        };
        opt<size> size;
        opt<::window::anchor> anchor;
        opt<::window::layer> layer;
        opt<::window::margin> margin;

        struct style {
            opt<float> padding;
            opt<float> border_radius;
            opt<float> fontsize;
            opt<std::string> font;
            opt<std::string> background_color;
        } style;

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