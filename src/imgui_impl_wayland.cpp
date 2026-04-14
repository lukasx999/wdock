#include "imgui_impl_wayland.hpp"
#include <print>

struct imgui_impl_wayland_data {
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
    .global_remove = [](auto...) { }
};

static wl_pointer_listener m_wl_pointer_listener {
    .enter = [](auto...) { },
    .leave = [](auto...) { },

    .motion = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
        double x = wl_fixed_to_double(surface_x);
        double y = wl_fixed_to_double(surface_y);
        ImGui::GetIO().AddMousePosEvent(x, y);
    },

    .button = [](void* data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
        // ImGui::GetIO().AddMouseButtonEvent(button, state);
    },

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