#pragma once

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include "imgui_impl_wayland.hpp"

#include "config.hpp"
#include "utils.hpp"

class ui {
    public:
    ui(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplWayland_Init(wl_display, wl_egl_window);
        ImGui_ImplOpenGL3_Init();

        configure();
    }

    ~ui() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWayland_Shutdown();
        ImGui::DestroyContext();
    }

    ui(const ui&) = delete;
    ui(ui&&) = delete;
    ui& operator=(const ui&) = delete;
    ui& operator=(ui&&) = delete;

    void draw(std::invocable auto fn) const {
        with_frame_context([&] {
            int flags = ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_NoScrollbar
                | ImGuiWindowFlags_NoScrollWithMouse
                | ImGuiWindowFlags_NoNav
                | ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoResize;

            ImGuiIO& io = ImGui::GetIO();
            ImGui::SetNextWindowPos({ 0, 0 });
            ImGui::SetNextWindowSize(io.DisplaySize);

            ImGui::Begin("main", nullptr, flags);
            fn();
            ImGui::End();

        });
    }

    void load_style(const struct config::window::style& style_config) {
        auto& style = ImGui::GetStyle();

        float spacing = style_config.item_spacing;
        style.ItemSpacing = ImVec2(spacing, spacing);

        float padding = style_config.padding;
        style.WindowPadding = ImVec2(padding, padding);

        style.WindowRounding = style_config.border_radius;

        auto color_bg = parse_color_string(style_config.background_color);
        if (not color_bg)
            throw config_error("ERROR: failed to parse color \"{}\"", style_config.background_color);

        style.Colors[ImGuiCol_WindowBg] = *color_bg;

        style.FontSizeBase = style_config.fontsize;

        const char* font_name = style_config.font.c_str();
        auto font = parse_font_name(font_name);
        if (!font)
            throw config_error("ERROR: failed to parse font \"{}\"", font_name);

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->ClearFonts();
        io.Fonts->AddFontFromFileTTF(font->c_str());
    }

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

};