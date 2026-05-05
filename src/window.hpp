#pragma once

#include <cassert>
#include <memory>
#include <span>
#include <functional>
#include <stdexcept>

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
    enum class anchor {
        top,
        right,
        bottom,
        left,
    };

    enum class layer {
        background,
        bottom,
        top,
        overlay,
    };

    struct margin {
        int top    = 0;
        int right  = 0;
        int bottom = 0;
        int left   = 0;
    };

    window(const char* title, int width, int height);

    ~window();
    window(const window&) = delete;
    window(window&&) noexcept = default;
    window& operator=(const window&) = delete;
    window& operator=(window&&) noexcept = delete;

    void run();
    static void run_concurrent(std::span<const window*> windows);

    void on_draw(std::function<void()> draw_callback) {
        m_state->draw_callback = draw_callback;
    }

    [[nodiscard]] int get_width() const {
        int width;
        wl_egl_window_get_attached_size(m_state->wl_egl_window, &width, nullptr);
        return width;
    }

    [[nodiscard]] int get_height() const {
        int height;
        wl_egl_window_get_attached_size(m_state->wl_egl_window, nullptr, &height);
        return height;
    }

    void set_size(int width, int height) {
        zwlr_layer_surface_v1_set_size(m_state->zwlr_layer_surface, width, height);
    }

    void set_anchor(anchor anchor) {
        zwlr_layer_surface_v1_set_anchor(m_state->zwlr_layer_surface, anchor_to_wlr_anchor(anchor));
    }

    void set_margin(margin margin) {
        zwlr_layer_surface_v1_set_margin(m_state->zwlr_layer_surface, margin.top, margin.right, margin.bottom, margin.left);
    }

    void set_layer(layer layer) {
        zwlr_layer_surface_v1_set_layer(m_state->zwlr_layer_surface, layer_to_wlr_layer(layer));
    }

    [[nodiscard]] struct wl_display* get_wl_display() const {
        return m_state->wl_display;
    }

    [[nodiscard]] struct wl_egl_window* get_wl_egl_window() const {
        return m_state->wl_egl_window;
    }

    private:
    struct state {
        struct wl_display*    wl_display    = nullptr;
        struct wl_surface*    wl_surface    = nullptr;
        struct wl_registry*   wl_registry   = nullptr;
        struct wl_compositor* wl_compositor = nullptr;

        struct zwlr_layer_shell_v1*   zwlr_layer_shell   = nullptr;
        struct zwlr_layer_surface_v1* zwlr_layer_surface = nullptr;

        struct wl_egl_window* wl_egl_window = nullptr;

        EGLDisplay egl_display = nullptr;
        EGLSurface egl_surface = nullptr;
        EGLContext egl_context = nullptr;
        EGLConfig  egl_config  = nullptr;

        std::function<void()> draw_callback = [] { };
        bool should_close = false;
    };

    // we need to use unique_ptr here to make the class movable, as otherwise, if a window
    // is moved from, the new owner will still have pointers to the state in the old,
    // moved-from object. (the void* arg in the wayland event listeners)
    // hence we need to allocate this struct on the heap.
    std::unique_ptr<state> m_state;

    [[nodiscard]] static zwlr_layer_surface_v1_anchor anchor_to_wlr_anchor(anchor anchor);
    [[nodiscard]] static zwlr_layer_shell_v1_layer layer_to_wlr_layer(layer layer);
    [[nodiscard]] bool init_egl(int width, int height);
    void setup_layer_surface(const char* title, int width, int height);
    static void bind_globals(void* data, struct wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version);
    static void configure_layer_surface(void* data, struct zwlr_layer_surface_v1* zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height);
    static void draw_frame(void* data, struct wl_callback* wl_callback, uint32_t callback_data);
    static void configure_toplevel(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, struct wl_array* states);

    static inline xdg_toplevel_listener m_xdg_toplevel_listener {
        .configure = configure_toplevel,
        .close = [](void* data, [[maybe_unused]] struct xdg_toplevel* xdg_toplevel) {
            state& state = *static_cast<struct state*>(data);
            state.should_close = true;
        },
        .configure_bounds = [](auto...) { },
        .wm_capabilities = [](auto...) { },
    };

    static inline xdg_surface_listener m_xdg_surface_listener {
        .configure = []([[maybe_unused]] void* data, struct xdg_surface* xdg_surface, uint32_t serial) {
            xdg_surface_ack_configure(xdg_surface, serial);
        }
    };

    static inline wl_registry_listener m_registry_listener {
        .global = bind_globals,
        .global_remove = [](auto...) { },
    };

    static inline zwlr_layer_surface_v1_listener m_layer_surface_listener {
        .configure = configure_layer_surface,
        .closed = [](auto...) { },
    };

    static inline wl_callback_listener m_frame_callback_listener {
        .done = draw_frame,
    };

};