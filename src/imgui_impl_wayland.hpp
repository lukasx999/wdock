#pragma once

#include <functional>
#include <string_view>

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <wayland-egl.h>
#include <wayland-client.h>
#include <linux/input-event-codes.h>

struct imgui_impl_wayland_data {
    ImGuiIO* io;
    struct wl_display* wl_display = nullptr;
    struct wl_egl_window* wl_egl_window = nullptr;
    struct wl_registry* wl_registry = nullptr;
    struct wl_seat* wl_seat = nullptr;
    struct wl_pointer* wl_pointer = nullptr;
};

[[nodiscard]] inline imgui_impl_wayland_data& get_data() {
    return *static_cast<imgui_impl_wayland_data*>(ImGui::GetIO().BackendPlatformUserData);
}

inline void bind_globals(void* data, struct wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version) {
    imgui_impl_wayland_data& state = *static_cast<imgui_impl_wayland_data*>(data);

    using namespace std::placeholders;
    auto bind_global = std::bind(wl_registry_bind, wl_registry, name, _1, version);

    if (std::string_view(interface) == wl_seat_interface.name)
        state.wl_seat = static_cast<wl_seat*>(bind_global(&wl_seat_interface));

}

static wl_registry_listener m_registry_listener {
    .global = bind_globals,
    .global_remove = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_registry* wl_registry, [[maybe_unused]] uint32_t name) { }
};

static wl_pointer_listener m_wl_pointer_listener {
    .enter = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t serial, [[maybe_unused]] struct wl_surface* surface, [[maybe_unused]] wl_fixed_t surface_x, [[maybe_unused]] wl_fixed_t surface_y) { },
    .leave = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t serial, [[maybe_unused]] struct wl_surface* surface) { },

    .motion = [](void* data, struct wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
        imgui_impl_wayland_data& state = *static_cast<imgui_impl_wayland_data*>(data);
        state.io->AddMousePosEvent(surface_x, surface_y);
    },

    .button = [](void* data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
        // imgui_impl_wayland_data& wdata = *static_cast<imgui_impl_wayland_data*>(data);
        // wdata.io->AddMouseButtonEvent(button, state);
    },

    .axis = [](auto...) { },
    .frame = [](auto...) { },
    .axis_source = [](auto...) { },
    .axis_stop = [](auto...) { },
    .axis_discrete = [](auto...) { },
    .axis_value120 = [](auto...) { },
    .axis_relative_direction = [](auto...) { },
};

void ImGui_ImplWayland_Init(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window);
void ImGui_ImplWayland_Shutdown();
void ImGui_ImplWayland_NewFrame();