#include "widget.hpp"

void widget::apply_style() const {
    auto& style = ImGui::GetStyle();

    style.FrameRounding = m_style.frame_rounding;
    style.FramePadding = ImVec2(m_style.frame_padding, m_style.frame_padding);

    set_color(ImGuiCol_Text, m_style.color_text);
    set_color(ImGuiCol_PlotHistogram, m_style.color_progress);
    set_color(ImGuiCol_FrameBg, m_style.color_frame_bg);
    set_color(ImGuiCol_Button, m_style.color_button);
    set_color(ImGuiCol_ButtonActive, m_style.color_button_active);
    set_color(ImGuiCol_ButtonHovered, m_style.color_button_hovered);
}

void widget::set_color(ImGuiCol imgui_color, std::string_view color_string) const {

    auto color = parse_color_string(color_string);
    if (not color)
        throw widget_error("failed to parse color \"{}\"", color_string);

    auto& style = ImGui::GetStyle();
    style.Colors[imgui_color] = *color;

}