#pragma once

#include <functional>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

struct window_style {
    float padding                = 20.0f;
    float border_radius          = 15.0f;
    float fontsize               = 30.0f;
    float item_spacing           = 10.0f;
    std::string font             = "JetBrainsMonoNerdFontMono";
    std::string background_color = "#0000007f";
};

class ui {
    public:
    ui(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window);

    ~ui();
    ui(const ui&) = delete;
    ui(ui&&) = delete;
    ui& operator=(const ui&) = delete;
    ui& operator=(ui&&) = delete;

    void draw(std::function<void()> fn) const;
    void load_style(const window_style& window_style);

    private:
    void with_frame_context(std::invocable auto fn) const;
    void load_font(const char* font_name);

};