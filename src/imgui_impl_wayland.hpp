#pragma once

#include <functional>
#include <string_view>

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <wayland-egl.h>
#include <wayland-client.h>
#include <linux/input-event-codes.h>

void ImGui_ImplWayland_Init(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window);
void ImGui_ImplWayland_Shutdown();
void ImGui_ImplWayland_NewFrame();