#include "wayland.hpp"

wayland_layer_surface::wayland_layer_surface(int width, int height, const char* title, anchor anchor) {

    m_state.wl_display = wl_display_connect(nullptr);
    if (m_state.wl_display == nullptr)
        throw wayland_error("failed to connect to wayland display");

    m_state.wl_registry = wl_display_get_registry(m_state.wl_display);
    wl_registry_add_listener(m_state.wl_registry, &m_registry_listener, &m_state);
    wl_display_roundtrip(m_state.wl_display);

    m_state.wl_surface = wl_compositor_create_surface(m_state.wl_compositor);

    if (!init_egl(width, height))
        throw wayland_error("failed to initialize EGL");

    setup_layer_surface(width, height, title, anchor);

    wl_surface_commit(m_state.wl_surface);
    wl_display_roundtrip(m_state.wl_display);

    wl_callback* frame_callback = wl_surface_frame(m_state.wl_surface);
    wl_callback_add_listener(frame_callback, &m_frame_callback_listener, &m_state);

    eglSwapBuffers(m_state.egl_display, m_state.egl_surface);

}

bool wayland_layer_surface::init_egl(int width, int height) {

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

    m_state.egl_display = eglGetDisplay(m_state.wl_display);
    if (m_state.egl_display == EGL_NO_DISPLAY) return false;

    EGLint major, minor;
    if (eglInitialize(m_state.egl_display, &major, &minor) != EGL_TRUE) return false;

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

    return true;
}

void wayland_layer_surface::bind_globals(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version) {
    state& state = *static_cast<struct state*>(data);

    using namespace std::placeholders;
    auto bind_global = std::bind(wl_registry_bind, wl_registry, name, _1, version);

    if (std::string_view(interface) == wl_compositor_interface.name)
        state.wl_compositor = static_cast<wl_compositor*>(bind_global(&wl_compositor_interface));

    if (std::string_view(interface) == zwlr_layer_shell_v1_interface.name)
        state.zwlr_layer_shell = static_cast<zwlr_layer_shell_v1*>(bind_global(&zwlr_layer_shell_v1_interface));

}

void wayland_layer_surface::draw_frame(void* data, struct wl_callback* wl_callback, [[maybe_unused]] uint32_t callback_data) {
    state& state = *static_cast<struct state*>(data);

    wl_callback_destroy(wl_callback);

    struct wl_callback* frame_callback = wl_surface_frame(state.wl_surface);
    wl_callback_add_listener(frame_callback, &m_frame_callback_listener, &state);

    state.draw_callback();
    eglSwapBuffers(state.egl_display, state.egl_surface);
}

void wayland_layer_surface::configure_surface(void* data, struct zwlr_layer_surface_v1* zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height) {
    zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);
    state& state = *static_cast<struct state*>(data);
    glViewport(0, 0, width, height);
    wl_egl_window_resize(state.egl_window, width, height, 0, 0);
}