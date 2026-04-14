#include "imgui_impl_wayland.hpp"

void ImGui_ImplWayland_Init(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window) {
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_wayland";
    io.BackendPlatformUserData = IM_NEW(imgui_impl_wayland_data)();

    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    auto& data = get_data();
    data.wl_display = wl_display;
    data.wl_egl_window = wl_egl_window;
    data.io = &io;

    data.wl_registry = wl_display_get_registry(data.wl_display);
    wl_registry_add_listener(data.wl_registry, &m_registry_listener, &data);
    wl_display_roundtrip(data.wl_display);

    data.wl_pointer = wl_seat_get_pointer(data.wl_seat);
    wl_pointer_add_listener(data.wl_pointer, &m_wl_pointer_listener, &data);
}

void ImGui_ImplWayland_Shutdown() {

    ImGuiIO& io = ImGui::GetIO();
    auto& data = get_data();

    io.BackendFlags &= ~ImGuiBackendFlags_HasMouseCursors;

    IM_DELETE(&data);
    io.BackendPlatformUserData = nullptr;
    io.BackendPlatformName = nullptr;
}

void ImGui_ImplWayland_NewFrame() {
    auto& data = get_data();

    int width, height;
    wl_egl_window_get_attached_size(data.wl_egl_window, &width, &height);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
    io.DeltaTime = 1.0f / 60.0f;
}