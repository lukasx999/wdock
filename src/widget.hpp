#pragma once

#include <stdexcept>
#include <format>

#include "imgui.h"
#include "imgui_stdlib.h"

#include "utils.hpp"

struct widget_error : std::runtime_error {
    using std::runtime_error::runtime_error;

    template <typename... Args>
    widget_error(std::format_string<Args...> fmt, Args&&... args)
    : widget_error(std::format(fmt, std::forward<Args>(args)...))
    { }
};

struct widget_style {
    std::string color_frame_bg       = "#2e3440";
    std::string color_text           = "#eceff4";
    std::string color_button         = "#2e3440";
    std::string color_button_hovered = "#3b4252";
    std::string color_button_active  = "#434c5e";
    std::string color_progress       = "#4c566a";
    float frame_padding = 5;
    float frame_rounding = 5;
    // TODO:
    // bool centered = false;
};

class widget {
    public:
    explicit widget(widget_style style)
    : m_style(style)
    { }

    virtual ~widget() = default;

    void draw() const {
        apply_style();
        on_draw();
    }

    protected:
    const widget_style m_style;

    virtual void on_draw() const { };
    void apply_style() const;

    private:
    void set_color(ImGuiCol imgui_color, std::string_view color_string) const;

};
