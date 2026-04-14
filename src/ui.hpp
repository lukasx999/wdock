#pragma once

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include "imgui_impl_wayland.hpp"

struct window_style {
    float padding = 25.0f;
    float rounding = 15.0f;
    float font_size = 30.0f;
    std::string_view font_family;
    ImVec4 bg_color = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
};

class ui {
    public:
    ui(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplWayland_Init(wl_display, wl_egl_window);
        ImGui_ImplOpenGL3_Init();
        configure();
        set_style();
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

    private:
    void set_style() const {
        auto& style = ImGui::GetStyle();

        float padding = 25;
        style.WindowPadding = ImVec2(padding, padding);
        style.WindowRounding = 15.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        style.FontSizeBase = 30.0f;
    }

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

        auto font_path = "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf";
        io.Fonts->AddFontFromFileTTF(font_path);
    }

};