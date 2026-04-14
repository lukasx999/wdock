#pragma once

#include <imgui.h>

#include <wayland-client.h>
#include <wayland-egl.h>

void ImGui_ImplWayland_Init(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window);
void ImGui_ImplWayland_Shutdown();
void ImGui_ImplWayland_NewFrame();