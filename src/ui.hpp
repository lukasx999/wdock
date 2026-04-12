#pragma once

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_null.h>
#include <imgui_stdlib.h>

#include "imgui_impl_wayland.hpp"

class ui {
    public:
    ui() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplWayland_Init();
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

    void draw(int width, int height, std::invocable auto fn) const {
        with_frame_context([&] {
            int flags = ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_NoMove;

            set_next_window_dimensions();

            float padding = 25;
            auto& style = ImGui::GetStyle();
            style.WindowPadding = ImVec2(padding, padding);
            style.WindowRounding = 15.0f;
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.75f));
            ImGui::Begin("main", nullptr, flags);
            fn();
            ImGui::End();
            ImGui::PopStyleColor();

        }, width, height);
    }

    private:
    void with_frame_context(std::invocable auto fn, int width, int height) const {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWayland_NewFrame(width, height);

        ImGui::NewFrame();
        fn();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void configure() const {
        ImGuiStyle& style = ImGui::GetStyle();
        style.FontSizeBase = 40.0f;

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        auto font_path = "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf";
        io.Fonts->AddFontFromFileTTF(font_path);
    }

    void set_next_window_dimensions() const {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::SetNextWindowSize(io.DisplaySize);
    }

};