#pragma once

#include <cassert>
#include <functional>
#include <stdexcept>

#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <wayland-client.h>

#include <wlr-layer-shell-unstable-v1.h>
#include <xdg-shell.h>

#include <glad/gl.h>
#include <glad/egl.h>

struct window_error : std::runtime_error {
    using runtime_error::runtime_error;
};

class window {
    public:
    enum class anchor { top, bottom, left, right };

    struct margin {
        int top    = 0;
        int right  = 0;
        int bottom = 0;
        int left   = 0;
    };

    window(int width, int height, const char* title, anchor anchor, margin margin={0, 0, 0, 0});

    ~window();
    window(const window&) = delete;
    window(window&&) = delete;
    window& operator=(const window&) = delete;
    window& operator=(window&&) = delete;

    void on_draw(std::function<void()> draw_callback) {
        m_state.draw_callback = draw_callback;
    }

    [[nodiscard]] int get_width() const {
        int width;
        wl_egl_window_get_attached_size(m_state.egl_window, &width, nullptr);
        return width;
    }

    [[nodiscard]] int get_height() const {
        int height;
        wl_egl_window_get_attached_size(m_state.egl_window, nullptr, &height);
        return height;
    }

    void run() const {
        while (wl_display_dispatch(m_state.wl_display) != -1);
    }

    bool run_async() const {
        bool should_quit = wl_display_dispatch(m_state.wl_display) == -1;
        return should_quit;
    }

    private:

    struct state {
        struct wl_display*    wl_display    = nullptr;
        struct wl_surface*    wl_surface    = nullptr;
        struct wl_registry*   wl_registry   = nullptr;
        struct wl_compositor* wl_compositor = nullptr;
        struct wl_seat*       wl_seat       = nullptr;
        struct wl_pointer*    wl_pointer    = nullptr;

        struct xdg_wm_base*  xdg_wm_base  = nullptr;
        struct xdg_surface*  xdg_surface  = nullptr;
        struct xdg_toplevel* xdg_toplevel = nullptr;

        struct zwlr_layer_shell_v1*   zwlr_layer_shell   = nullptr;
        struct zwlr_layer_surface_v1* zwlr_layer_surface = nullptr;

        struct wl_egl_window* egl_window = nullptr;

        EGLDisplay egl_display = nullptr;
        EGLSurface egl_surface = nullptr;
        EGLContext egl_context = nullptr;
        EGLConfig  egl_config  = nullptr;

        std::function<void()> draw_callback;
    };

    state m_state;

    [[nodiscard]] static zwlr_layer_surface_v1_anchor anchor_to_wlr_anchor(anchor anchor) {
        switch (anchor) {
            case anchor::top:    return ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
            case anchor::bottom: return ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
            case anchor::left:   return ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
            case anchor::right:  return ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
            default: assert(!"unreachable");
        }
    }

    [[nodiscard]] bool init_egl(int width, int height);
    void setup_toplevel(const char* title);
    void setup_layer_surface(int width, int height, const char* title, anchor anchor, margin margin);
    static void bind_globals(void* data, struct wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version);
    static void configure_layer_surface(void* data, struct zwlr_layer_surface_v1* zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height);
    static void draw_frame(void* data, struct wl_callback* wl_callback, uint32_t callback_data);
    static void configure_toplevel(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, struct wl_array* states);

    static inline wl_pointer_listener m_wl_pointer_listener {
        .enter = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t serial, [[maybe_unused]] struct wl_surface* surface, [[maybe_unused]] wl_fixed_t surface_x, [[maybe_unused]] wl_fixed_t surface_y) { },
        .leave = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t serial, [[maybe_unused]] struct wl_surface* surface) { },

        .motion = [](void* data, struct wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {

        },

        .button = [](void* data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {

        },

        .axis = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t time, [[maybe_unused]] uint32_t axis, [[maybe_unused]] wl_fixed_t value) { },
        .frame = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer) { },
        .axis_source = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t axis_source) { },
        .axis_stop = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t time, [[maybe_unused]] uint32_t axis) { },
        .axis_discrete = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t axis, [[maybe_unused]] int32_t discrete) { },
        .axis_value120 = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t axis, [[maybe_unused]] int32_t value120) { },
        .axis_relative_direction = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_pointer* wl_pointer, [[maybe_unused]] uint32_t axis, [[maybe_unused]] uint32_t direction) { },
    };

    static inline xdg_wm_base_listener m_xdg_wm_base_listener {
        .ping = []([[maybe_unused]] void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial) {
            xdg_wm_base_pong(xdg_wm_base, serial);
        }
    };

    static inline xdg_toplevel_listener m_xdg_toplevel_listener {
        .configure = configure_toplevel,
        .close = []([[maybe_unused]] void* data, [[maybe_unused]] struct xdg_toplevel* xdg_toplevel) { },
        .configure_bounds = []([[maybe_unused]] void* data, [[maybe_unused]] struct xdg_toplevel* xdg_toplevel, [[maybe_unused]] int32_t width, [[maybe_unused]] int32_t height) { },
        .wm_capabilities = []([[maybe_unused]] void* data, [[maybe_unused]] struct xdg_toplevel* xdg_toplevel, [[maybe_unused]] struct wl_array* capabilities) { },
    };

    static inline xdg_surface_listener m_xdg_surface_listener {
        .configure = []([[maybe_unused]] void* data, struct xdg_surface* xdg_surface, uint32_t serial) {
            xdg_surface_ack_configure(xdg_surface, serial);
        }
    };

    static inline wl_registry_listener m_registry_listener {
        .global = bind_globals,
        .global_remove = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_registry* wl_registry, [[maybe_unused]] uint32_t name) { }
    };

    static inline zwlr_layer_surface_v1_listener m_layer_surface_listener {
        .configure = configure_layer_surface,
        .closed = []([[maybe_unused]] void* data, [[maybe_unused]] struct zwlr_layer_surface_v1* zwlr_layer_surface_v1) { }
    };

    static inline wl_callback_listener m_frame_callback_listener {
        .done = draw_frame,
    };

};