#include "window.hpp"
#include <wayland-client-protocol.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLAD_EGL_IMPLEMENTATION
#include <glad/egl.h>

window::window(int width, int height, const char* title, anchor anchor, margin margin) {

    m_state.wl_display = wl_display_connect(nullptr);
    if (m_state.wl_display == nullptr)
        throw window_error("failed to connect to wayland display");

    m_state.wl_registry = wl_display_get_registry(m_state.wl_display);
    wl_registry_add_listener(m_state.wl_registry, &m_registry_listener, &m_state);
    wl_display_roundtrip(m_state.wl_display);

    m_state.wl_pointer =  wl_seat_get_pointer(m_state.wl_seat);
    wl_pointer_add_listener(m_state.wl_pointer, &m_wl_pointer_listener, &m_state);

    m_state.wl_surface = wl_compositor_create_surface(m_state.wl_compositor);

    if (!init_egl(width, height))
        throw window_error("failed to initialize EGL");

    setup_layer_surface(width, height, title, anchor, margin);
    // setup_toplevel(title);

    wl_surface_commit(m_state.wl_surface);
    wl_display_roundtrip(m_state.wl_display);

    wl_callback* frame_callback = wl_surface_frame(m_state.wl_surface);
    wl_callback_add_listener(frame_callback, &m_frame_callback_listener, &m_state);

    eglSwapBuffers(m_state.egl_display, m_state.egl_surface);
}

window::~window() {
    gladLoaderUnloadGL();

    eglDestroyContext(m_state.egl_display, m_state.egl_context);
    eglDestroySurface(m_state.egl_display, m_state.egl_surface);
    eglTerminate(m_state.egl_display);

    gladLoaderUnloadEGL();

    wl_display_disconnect(m_state.wl_display);
}

bool window::init_egl(int width, int height) {

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

    // for some reason we need to call gladLoaderLoadEGL() before and after eglInitialize().
    if (!gladLoaderLoadEGL(nullptr)) return false;

    m_state.egl_display = eglGetDisplay(m_state.wl_display);
    if (m_state.egl_display == EGL_NO_DISPLAY) return false;

    EGLint major, minor;
    if (eglInitialize(m_state.egl_display, &major, &minor) != EGL_TRUE) return false;

    if (!gladLoaderLoadEGL(m_state.egl_display)) return false;

    EGLint config_count;
    eglGetConfigs(m_state.egl_display, nullptr, 0, &config_count);

    std::vector<EGLConfig> configs(config_count);

    if (!eglBindAPI(EGL_OPENGL_API)) return false;

    EGLint n;
    eglChooseConfig(m_state.egl_display, config_attribs.data(), configs.data(), config_count, &n);

    m_state.egl_config = configs.front();
    m_state.egl_context = eglCreateContext(m_state.egl_display, m_state.egl_config, EGL_NO_CONTEXT, context_attribs.data());
    if (m_state.egl_context == EGL_NO_CONTEXT) return false;

    m_state.egl_window = wl_egl_window_create(m_state.wl_surface, width, height);
    if (m_state.egl_window == EGL_NO_SURFACE) return false;

    m_state.egl_surface = eglCreateWindowSurface(m_state.egl_display, m_state.egl_config, m_state.egl_window, nullptr);
    if (!eglMakeCurrent(m_state.egl_display, m_state.egl_surface, m_state.egl_surface, m_state.egl_context)) return false;

    gladLoaderLoadGL();

    return true;
}

void window::setup_toplevel(const char* title) {

    xdg_wm_base_add_listener(m_state.xdg_wm_base, &m_xdg_wm_base_listener, nullptr);

    m_state.xdg_surface = xdg_wm_base_get_xdg_surface(m_state.xdg_wm_base, m_state.wl_surface);
    m_state.xdg_toplevel = xdg_surface_get_toplevel(m_state.xdg_surface);
    xdg_toplevel_set_title(m_state.xdg_toplevel, title);

    xdg_toplevel_add_listener(m_state.xdg_toplevel, &m_xdg_toplevel_listener, this);
    xdg_surface_add_listener(m_state.xdg_surface, &m_xdg_surface_listener, nullptr);

}

void window::setup_layer_surface(int width, int height, const char* title, anchor anchor, margin margin) {

    m_state.zwlr_layer_surface = zwlr_layer_shell_v1_get_layer_surface(m_state.zwlr_layer_shell, m_state.wl_surface,
        nullptr, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, title);

    zwlr_layer_surface_v1_add_listener(m_state.zwlr_layer_surface, &m_layer_surface_listener, &m_state);
    zwlr_layer_surface_v1_set_size(m_state.zwlr_layer_surface, width, height);
    zwlr_layer_surface_v1_set_anchor(m_state.zwlr_layer_surface, anchor_to_wlr_anchor(anchor));
    zwlr_layer_surface_v1_set_margin(m_state.zwlr_layer_surface, margin.top, margin.right, margin.bottom, margin.left);
}

void window::bind_globals(void* data, struct wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version) {
    state& state = *static_cast<struct state*>(data);

    using namespace std::placeholders;
    auto bind_global = std::bind(wl_registry_bind, wl_registry, name, _1, version);

    if (std::string_view(interface) == wl_compositor_interface.name)
        state.wl_compositor = static_cast<wl_compositor*>(bind_global(&wl_compositor_interface));

    if (std::string_view(interface) == wl_seat_interface.name)
        state.wl_seat = static_cast<wl_seat*>(bind_global(&wl_seat_interface));

    else if (std::string_view(interface) == zwlr_layer_shell_v1_interface.name)
        state.zwlr_layer_shell = static_cast<zwlr_layer_shell_v1*>(bind_global(&zwlr_layer_shell_v1_interface));

    else if (std::string_view(interface) == xdg_wm_base_interface.name)
        state.xdg_wm_base = static_cast<xdg_wm_base*>(bind_global(&xdg_wm_base_interface));

}

void window::draw_frame(void* data, struct wl_callback* wl_callback, [[maybe_unused]] uint32_t callback_data) {
    state& state = *static_cast<struct state*>(data);

    wl_callback_destroy(wl_callback);
    struct wl_callback* frame_callback = wl_surface_frame(state.wl_surface);
    wl_callback_add_listener(frame_callback, &m_frame_callback_listener, &state);

    eglMakeCurrent(state.egl_display, state.egl_surface, state.egl_surface, state.egl_context);

    state.draw_callback();
    eglSwapBuffers(state.egl_display, state.egl_surface);
}

void window::configure_layer_surface(void* data, struct zwlr_layer_surface_v1* zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height) {
    zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);
    state& state = *static_cast<struct state*>(data);
    glViewport(0, 0, width, height);
    wl_egl_window_resize(state.egl_window, width, height, 0, 0);
}

void window::configure_toplevel(void* data, [[maybe_unused]] struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, [[maybe_unused]] struct wl_array* states) {
    state& state = *static_cast<struct state*>(data);
    glViewport(0, 0, width, height);
    wl_egl_window_resize(state.egl_window, width, height, 0, 0);
}