#pragma once

#include <filesystem>

#include "window.hpp"
#include "widgets.hpp"

struct config_error : std::runtime_error {
    using std::runtime_error::runtime_error;

    template <typename... Args>
    config_error(std::format_string<Args...> fmt, Args... args)
    : config_error(std::vformat(fmt.get(), std::make_format_args(args...)))
    { }

};

// TODO: set default values in the code or always load a default config first?
struct config {
    struct window {
        struct size {
            int width = 700;
            int height = 800;
        };

        struct style {
            float padding = 20.0f;
            float border_radius = 15.0f;
            float fontsize = 30.0f;
            float item_spacing = 10.0f;
            std::string font = "JetBrainsMonoNerdFontMono";
            // TODO: add parsing for css colors
            std::string background_color = "#0000007f";
        };

        size size;
        ::window::anchor anchor = ::window::anchor::right;
        ::window::layer layer = ::window::layer::background;
        ::window::margin margin = { 0, 200, 0, 0 };
        style style;
    };

    window window;
    std::vector<std::unique_ptr<widget>> widgets;

};

[[nodiscard]] config parse_config(const std::filesystem::path& config_path);