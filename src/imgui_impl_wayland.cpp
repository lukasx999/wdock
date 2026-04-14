#include <print>
#include <functional>
#include <string_view>

#include <linux/input-event-codes.h>

#include "imgui_impl_wayland.hpp"

struct imgui_impl_wayland_data {
    struct wl_display* wl_display = nullptr;
    struct wl_egl_window* wl_egl_window = nullptr;
    struct wl_registry* wl_registry = nullptr;
    struct wl_seat* wl_seat = nullptr;
    struct wl_pointer* wl_pointer = nullptr;
};

[[nodiscard]] static imgui_impl_wayland_data& get_data() {
    return *static_cast<imgui_impl_wayland_data*>(ImGui::GetIO().BackendPlatformUserData);
}

static void bind_globals([[maybe_unused]] void* data, struct wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version) {
    auto& state = get_data();

    using namespace std::placeholders;
    auto bind_global = std::bind(wl_registry_bind, wl_registry, name, _1, version);

    if (std::string_view(interface) == wl_seat_interface.name)
        state.wl_seat = static_cast<wl_seat*>(bind_global(&wl_seat_interface));

}

static void handle_pointer_button([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t serial, [[maybe_unused]] uint32_t time, uint32_t button, uint32_t state) {
    int imgui_button = [&]() {
        switch (button) {
            case BTN_LEFT: return 0;
            case BTN_RIGHT: return 1;
            case BTN_MIDDLE: return 2;
            default: return -1;
        }
    }();

    if (imgui_button != -1)
        ImGui::GetIO().AddMouseButtonEvent(imgui_button, state);

}

static void handle_pointer_motion([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    double x = wl_fixed_to_double(surface_x);
    double y = wl_fixed_to_double(surface_y);
    ImGui::GetIO().AddMousePosEvent(x, y);
}

static wl_registry_listener registry_listener {
    .global = bind_globals,
    .global_remove = [](auto...) { }
};

static wl_pointer_listener pointer_listener {
    .enter = [](auto...) { },
    .leave = [](auto...) { },
    .motion = handle_pointer_motion,
    .button =  handle_pointer_button,
    .axis = [](auto...) { },
    .frame = [](auto...) { },
    .axis_source = [](auto...) { },
    .axis_stop = [](auto...) { },
    .axis_discrete = [](auto...) { },
    .axis_value120 = [](auto...) { },
    .axis_relative_direction = [](auto...) { },
};

void ImGui_ImplWayland_Init(struct wl_display* wl_display, struct wl_egl_window* wl_egl_window) {
    IM_ASSERT(wl_display != nullptr);
    IM_ASSERT(wl_egl_window != nullptr);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_wayland";
    io.BackendPlatformUserData = IM_NEW(imgui_impl_wayland_data)();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    auto& data = get_data();
    data.wl_display = wl_display;
    data.wl_egl_window = wl_egl_window;

    data.wl_registry = wl_display_get_registry(data.wl_display);
    wl_registry_add_listener(data.wl_registry, &registry_listener, nullptr);
    wl_display_roundtrip(data.wl_display);

    data.wl_pointer = wl_seat_get_pointer(data.wl_seat);
    wl_pointer_add_listener(data.wl_pointer, &pointer_listener, nullptr);
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