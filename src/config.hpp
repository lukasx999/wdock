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

struct config {
    private:
    template <typename T> using opt = std::optional<T>;

    public:
    struct window {
        struct size {
            int width, height;
        };

        struct style {
            opt<float> padding;
            opt<float> border_radius;
            opt<float> fontsize;
            opt<std::string> font;
            opt<std::string> background_color;
        };

        opt<size> size;
        opt<::window::anchor> anchor;
        opt<::window::layer> layer;
        opt<::window::margin> margin;
        style style;
    };

    window window;
    std::vector<std::unique_ptr<widget>> widgets;

};

[[nodiscard]] config parse_config(const std::filesystem::path& config_path);