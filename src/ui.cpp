#include "ui.hpp"

ui::ui(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWayland_Init(wl_display, wl_egl_window);
    ImGui_ImplOpenGL3_Init();

    configure();
}

ui::~ui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWayland_Shutdown();
    ImGui::DestroyContext();
}

void ui::draw(std::function<void()> fn) const {
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

void ui::load_style(const struct config::window::style& style_config) {
    auto& style = ImGui::GetStyle();

    float spacing = style_config.item_spacing;
    style.ItemSpacing = ImVec2(spacing, spacing);

    float padding = style_config.padding;
    style.WindowPadding = ImVec2(padding, padding);

    style.WindowRounding = style_config.border_radius;

    auto color_bg = parse_color_string(style_config.background_color);
    if (not color_bg)
        throw config_error("failed to parse color \"{}\"", style_config.background_color);

    style.Colors[ImGuiCol_WindowBg] = *color_bg;

    style.FontSizeBase = style_config.fontsize;
    load_font(style_config.font.c_str());

}