#pragma once

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include "imgui_impl_wayland.hpp"

#include "config.hpp"
#include "utils.hpp"

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
    void with_frame_context(std::invocable auto fn) const {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWayland_NewFrame();
        ImGui::NewFrame();
        fn();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void configure() const {
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
    }

    void load_font(const char* font_name) {

        auto font = parse_font_name(font_name);
        if (!font)
            throw config_error("failed to parse font name \"{}\"", font_name);
        print_debug("loaded font from \"{}\"", font->string());

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->ClearFonts();

        auto ret = io.Fonts->AddFontFromFileTTF(font->c_str());
        if (ret == nullptr)
            throw config_error("failed to load font \"{}\"", font_name);

    }

};