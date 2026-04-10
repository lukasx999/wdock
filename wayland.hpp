#pragma once

#include <functional>
#include <stdexcept>
#include <string_view>

#include <wayland-client-core.h>
#include <wayland-egl.h>

#undef USE_GLAD

#ifdef USE_GLAD
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#ifdef USE_GLAD
#define GLAD_EGL_IMPLEMENTATION
#include <glad/egl.h>
#else
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1.h"

class wayland_layer_shell {

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
    };

    public:
    wayland_layer_shell(int width, int height, const char* title)
    {

        m_state.wl_display = wl_display_connect(nullptr);
        m_state.wl_registry = wl_display_get_registry(m_state.wl_display);
        wl_registry_add_listener(m_state.wl_registry, &wl_registry_listener_, &m_state);
        wl_display_roundtrip(m_state.wl_display);

        m_state.wl_surface = wl_compositor_create_surface(m_state.wl_compositor);

        if (!init_egl(width, height, m_state))
            throw std::runtime_error("failed to initialize EGL");

        m_state.zwlr_layer_surface = zwlr_layer_shell_v1_get_layer_surface(m_state.zwlr_layer_shell, m_state.wl_surface, nullptr, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, title);

        zwlr_layer_surface_v1_add_listener(m_state.zwlr_layer_surface, &zwlr_layer_surface_v1_listener_, &m_state);
        zwlr_layer_surface_v1_set_size(m_state.zwlr_layer_surface, 500, 500);
        zwlr_layer_surface_v1_set_anchor(m_state.zwlr_layer_surface, ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
        zwlr_layer_surface_v1_set_margin(m_state.zwlr_layer_surface, 0, 300, 0, 0);

        wl_surface_commit(m_state.wl_surface);
        wl_display_roundtrip(m_state.wl_display);

        wl_callback* frame_callback = wl_surface_frame(m_state.wl_surface);
        wl_callback_add_listener(frame_callback, &wl_callback_listener_, &m_state);

        eglSwapBuffers(m_state.egl_display, m_state.egl_surface);


    }

    ~wayland_layer_shell() {
        wl_display_disconnect(m_state.wl_display);
    }

    wayland_layer_shell(const wayland_layer_shell&) = delete;
    wayland_layer_shell(wayland_layer_shell&&) = delete;
    wayland_layer_shell& operator=(const wayland_layer_shell&) = delete;
    wayland_layer_shell& operator=(wayland_layer_shell&&) = delete;

    void run() {
        while (wl_display_dispatch(m_state.wl_display) != -1);
    }

    private:
    state m_state;

    static void draw() {
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    [[nodiscard]] bool init_egl(int width, int height, struct state& state) {

        std::array config_attribs {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE,   8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE,  8,
            EGL_ALPHA_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
            EGL_NONE
        };

        std::array context_attribs {
            EGL_CONTEXT_MAJOR_VERSION, 4,
            EGL_CONTEXT_MINOR_VERSION, 6,
            EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
            EGL_NONE
        };

        state.egl_display = eglGetDisplay(state.wl_display);
        if (state.egl_display == EGL_NO_DISPLAY) return false;

        EGLint major, minor;
        if (eglInitialize(state.egl_display, &major, &minor) != EGL_TRUE) return false;

        EGLint config_count;
        eglGetConfigs(state.egl_display, nullptr, 0, &config_count);

        std::vector<EGLConfig> configs(config_count);

        if (!eglBindAPI(EGL_OPENGL_API)) return false;

        EGLint n;
        eglChooseConfig(state.egl_display, config_attribs.data(), configs.data(), config_count, &n);

        state.egl_config = configs.front();
        state.egl_context = eglCreateContext(state.egl_display, state.egl_config, EGL_NO_CONTEXT, context_attribs.data());
        if (state.egl_context == EGL_NO_CONTEXT) return false;

        state.egl_window = wl_egl_window_create(state.wl_surface, width, height);
        if (state.egl_window == EGL_NO_SURFACE) return false;

        state.egl_surface = eglCreateWindowSurface(state.egl_display, state.egl_config, state.egl_window, nullptr);
        if (!eglMakeCurrent(state.egl_display, state.egl_surface, state.egl_surface, state.egl_context)) return false;

        return true;
    }

    static inline wl_registry_listener wl_registry_listener_ {
        .global = [](void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version) {
            state& state = *static_cast<struct state*>(data);

            using namespace std::placeholders;
            auto bind_global = std::bind(wl_registry_bind, wl_registry, name, _1, version);

            if (std::string_view(interface) == wl_compositor_interface.name)
                state.wl_compositor = static_cast<wl_compositor*>(bind_global(&wl_compositor_interface));

            if (std::string_view(interface) == zwlr_layer_shell_v1_interface.name)
                state.zwlr_layer_shell = static_cast<zwlr_layer_shell_v1*>(bind_global(&zwlr_layer_shell_v1_interface));

        },
        .global_remove = []([[maybe_unused]] void* data, [[maybe_unused]] struct wl_registry* wl_registry, [[maybe_unused]] uint32_t name) { }
    };

    static inline zwlr_layer_surface_v1_listener zwlr_layer_surface_v1_listener_ {
        .configure = [] (void* data, struct zwlr_layer_surface_v1* zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height) {
            zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);
            state& state = *static_cast<struct state*>(data);
            glViewport(0, 0, width, height);
            wl_egl_window_resize(state.egl_window, width, height, 0, 0);
        },
        .closed = []([[maybe_unused]] void* data, [[maybe_unused]] struct zwlr_layer_surface_v1* zwlr_layer_surface_v1) { }
    };

    static inline wl_callback_listener wl_callback_listener_ {
        .done = [] (void* data, struct wl_callback* wl_callback, [[maybe_unused]] uint32_t callback_data) {
            state& state = *static_cast<struct state*>(data);

            wl_callback_destroy(wl_callback);

            struct wl_callback* frame_callback = wl_surface_frame(state.wl_surface);
            wl_callback_add_listener(frame_callback, &wl_callback_listener_, &state);

            draw();
            eglSwapBuffers(state.egl_display, state.egl_surface);
        }
    };

};