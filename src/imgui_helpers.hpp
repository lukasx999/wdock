#pragma once

#include "imgui.h"
#include "imgui_stdlib.h"

namespace ImGuiHelpers {

    inline void SameLineRightAligned(float width) {
        float offset = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width;
        ImGui::SameLine();
        ImGui::SetCursorPosX(offset);
    }

    inline void Center(float width, float alignment=0.5f) {
        float total_width = ImGui::GetContentRegionAvail().x;
        float offset = (total_width - width) * alignment;
        if (offset >= 0.0f)
            ImGui::SetCursorPosX(offset);
    }

} // namespace ImGuiHelpers