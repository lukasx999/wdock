#pragma once

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include "config.hpp"

class ui {
    public:
    ui(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window);
    ~ui();

    ui(const ui&) = delete;
    ui(ui&&) = delete;
    ui& operator=(const ui&) = delete;
    ui& operator=(ui&&) = delete;

    void draw(std::function<void()> fn) const;
    void load_style(const struct config::window::style& style_config);

    private:
    void with_frame_context(std::invocable auto fn) const;
    void load_font(const char* font_name);

};