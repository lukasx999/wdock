#pragma once

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_null.h>
#include <imgui_stdlib.h>

class ui {
    public:
    ui() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        // ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();
        configure();
    }

    ~ui() {
        ImGui_ImplOpenGL3_Shutdown();
        // ImGui_ImplGlfw_Shutdown();
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
            ImGui::Begin("main", nullptr, flags);
            fn();
            ImGui::End();
        }, width, height);
    }

    private:

    void with_frame_context(std::invocable auto fn, int width, int height) const {
        ImGui_ImplOpenGL3_NewFrame();

        // ImGui_ImplGlfw_NewFrame();
        // ImGui_ImplNullPlatform_NewFrame();

        // TODO:
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(width, height);
        io.DeltaTime = 1.0f / 60.0f;

        ImGui::NewFrame();
        fn();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void configure() const {
        ImGuiStyle& style = ImGui::GetStyle();
        style.FontSizeBase = 40.0f;

        ImGuiIO& io = ImGui::GetIO();
        auto font_path = "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf";
        io.Fonts->AddFontFromFileTTF(font_path);
    }

    void set_next_window_dimensions() const {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::SetNextWindowSize(io.DisplaySize);
    }

};