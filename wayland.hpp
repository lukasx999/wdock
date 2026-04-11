#pragma once

#include <cassert>
#include <functional>
#include <stdexcept>

#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

struct wayland_error : std::runtime_error {
    using runtime_error::runtime_error;
};

class wayland_layer_surface {

    struct state {
        struct wl_display*    wl_display    = nullptr;
        struct wl_surface*    wl_surface    = nullptr;
        struct wl_registry*   wl_registry   = nullptr;
        struct wl_compositor* wl_compositor = nullptr;

        struct zwlr_layer_shell_v1* zwlr_layer_shell = nullptr;
        struct zwlr_layer_surface_v1* zwlr_layer_surface = nullptr;

        struct wl_egl_window* egl_window = nullptr;

        EGLDisplay egl_display = nullptr;
        EGLSurface egl_surface = nullptr;
        EGLContext egl_context = nullptr;
        EGLConfig  egl_config  = nullptr;

        std::function<void()> draw_callback;
    };

    public:

    enum class anchor { top, bottom, left, right };

    struct margin {
        int top    = 0;
        int right  = 0;
        int bottom = 0;
        int left   = 0;
    };

    wayland_layer_surface(int width, int height, const char* title, anchor anchor, margin margin={0, 0, 0, 0});

    ~wayland_layer_surface() {
        wl_display_disconnect(m_state.wl_display);
    }

    wayland_layer_surface(const wayland_layer_surface&) = delete;
    wayland_layer_surface(wayland_layer_surface&&) = delete;
    wayland_layer_surface& operator=(const wayland_layer_surface&) = delete;
    wayland_layer_surface& operator=(wayland_layer_surface&&) = delete;

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

    void dispatch() {
        while (wl_display_dispatch(m_state.wl_display) != -1);
    }

    private:
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

    void setup_layer_surface(int width, int height, const char* title, anchor anchor, margin margin) {

        m_state.zwlr_layer_surface = zwlr_layer_shell_v1_get_layer_surface(m_state.zwlr_layer_shell, m_state.wl_surface,
            nullptr, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, title);

        zwlr_layer_surface_v1_add_listener(m_state.zwlr_layer_surface, &m_layer_surface_listener, &m_state);
        zwlr_layer_surface_v1_set_size(m_state.zwlr_layer_surface, width, height);
        zwlr_layer_surface_v1_set_anchor(m_state.zwlr_layer_surface, anchor_to_wlr_anchor(anchor));
        zwlr_layer_surface_v1_set_margin(m_state.zwlr_layer_surface, margin.top, margin.right, margin.bottom, margin.left);
    }

    [[nodiscard]] bool init_egl(int width, int height);

    static void bind_globals(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version);
    static void configure_surface(void* data, struct zwlr_layer_surface_v1* zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height);
    static void draw_frame(void* data, struct wl_callback* wl_callback, [[maybe_unused]] uint32_t callback_data);

    static inline wl_registry_listener m_registry_listener {
        .global = bind_globals,
        .global_remove = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_registry* wl_registry, [[maybe_unused]] uint32_t name) { }
    };

    static inline zwlr_layer_surface_v1_listener m_layer_surface_listener {
        .configure = configure_surface,
        .closed = []([[maybe_unused]] void* data, [[maybe_unused]] struct zwlr_layer_surface_v1* zwlr_layer_surface_v1) { }
    };

    static inline wl_callback_listener m_frame_callback_listener {
        .done = draw_frame,
    };

};