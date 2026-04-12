#pragma once

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_null.h>
#include <imgui_stdlib.h>

inline void ImGui_ImplWayland_Init() {
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_wayland";
}

inline void ImGui_ImplWayland_Shutdown() {
}

inline void ImGui_ImplWayland_NewFrame(int width, int height) {
    ImGuiIO& io = ImGui::GetIO();
    // TODO: get width and height from egl context
    io.DisplaySize = ImVec2(width, height);
    io.DeltaTime = 1.0f / 60.0f;
}