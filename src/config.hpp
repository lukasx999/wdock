#pragma once

#include <filesystem>

#include "window.hpp"
#include "widgets.hpp"
#include "ui.hpp"

struct config_error : std::runtime_error {
    using std::runtime_error::runtime_error;

    template <typename... Args>
    config_error(std::format_string<Args...> fmt, Args&&... args)
    : config_error(std::format(fmt, std::forward<Args>(args)...))
    { }

};

struct config {
    struct window {
        struct size {
            int width = 700;
            int height = 800;
        };

        size size;
        ::window::anchor anchor = ::window::anchor::right;
        ::window::layer layer = ::window::layer::background;
        ::window::margin margin = { 0, 200, 0, 0 };
        window_style style;
    };

    window window;
    std::vector<std::unique_ptr<widget>> widgets;

};

[[nodiscard]] config parse_config(const std::filesystem::path& config_path);