#include <print>
#include <cassert>
#include <chrono>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

// #define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>

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


#include <cairomm/surface.h>
#include <cairomm/context.h>

// #include <imgui.h>
// #include <imgui_impl_glfw.h>
// #include <imgui_impl_opengl3.h>
// #include <imgui_stdlib.h>

#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1.h"



namespace {

// [[nodiscard]] GLFWwindow* setup_glfw(int width, int height, const char* window_title) {
//     glfwSetErrorCallback([]([[maybe_unused]] int error_code, char const* desc) {
//         std::println(stderr, "glfw error: {}", desc);
//     });
//
//     glfwInit();
//     glfwWindowHint(GLFW_RESIZABLE, false);
//     glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
//     GLFWwindow* window = glfwCreateWindow(width, height, window_title, nullptr, nullptr);
//
//     glfwMakeContextCurrent(window);
//     gladLoadGL(glfwGetProcAddress);
//     glfwSwapInterval(1);
//
//     glfwSetFramebufferSizeCallback(
//         window, []([[maybe_unused]] GLFWwindow* win, int w, int h) {
//             glViewport(0, 0, w, h);
//         }
//     );
//
//     #if 0
//     glDebugMessageCallback([](
//         [[maybe_unused]] GLenum src,
//         [[maybe_unused]] GLenum type,
//         [[maybe_unused]] GLuint id,
//         [[maybe_unused]] GLenum severity,
//         [[maybe_unused]] GLsizei len,
//         const char* msg,
//         [[maybe_unused]] const void* args
//     ) { std::println(stderr, "opengl error: {}", msg); }, nullptr);
//     #endif
//
//     return window;
// }
//
// void imgui_init(GLFWwindow* window) {
//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO& io = ImGui::GetIO();
//     io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
//
//     ImGui_ImplGlfw_InitForOpenGL(window, true);
//     ImGui_ImplOpenGL3_Init();
// }
//
// void imgui_shutdown() {
//     ImGui_ImplOpenGL3_Shutdown();
//     ImGui_ImplGlfw_Shutdown();
//     ImGui::DestroyContext();
// }
//
// void with_imgui_frame_context(std::invocable auto fn) {
//     ImGui_ImplOpenGL3_NewFrame();
//     ImGui_ImplGlfw_NewFrame();
//     ImGui::NewFrame();
//     fn();
//     ImGui::Render();
//     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
// }
//
// void imgui_configure() {
//     ImGuiStyle& style = ImGui::GetStyle();
//     style.FontSizeBase = 40.0f;
//
//     ImGuiIO& io = ImGui::GetIO();
//     // auto font_path = "/usr/share/fonts/TTF/Roboto-Regular.ttf";
//     auto font_path = "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf";
//     io.Fonts->AddFontFromFileTTF(font_path);
// }
//
// void imgui_set_next_window_dimensions(GLFWwindow* window) {
//     int w, h;
//     glfwGetFramebufferSize(window, &w, &h);
//
//     ImGui::SetNextWindowPos({ 0, 0 });
//     ImGui::SetNextWindowSize(ImVec2(w, h));
// }
//
// [[nodiscard]] std::string get_time_string() {
//     auto now = std::chrono::system_clock::now();
//     std::chrono::zoned_time zt("Europe/Vienna", now);
//     return std::format("{:%H:%M}", zt);
// }
//
// [[nodiscard]] std::string get_date_string() {
//     auto now = std::chrono::system_clock::now();
//     std::chrono::zoned_time zt("Europe/Vienna", now);
//     return std::format("{:%d.%m.%Y}", zt);
// }
//
// [[nodiscard]] std::string get_uname_string() {
//     struct utsname buf;
//     assert(uname(&buf) == 0);
//     return std::format("{} {} {} {}", buf.sysname, buf.nodename, buf.release, buf.machine);
// }
//
// [[nodiscard]] float get_memory_usage() {
//     struct sysinfo buf;
//     assert(sysinfo(&buf) == 0);
//     return static_cast<float>(buf.totalram - buf.freeram) / buf.totalram;
// }
//
// void imgui_draw() {
//
//     ImGui::Text("welcome to wdock");
//     ImGui::Text("%s", get_uname_string().c_str());
//     ImGui::Text(" %s", get_time_string().c_str());
//     ImGui::Text(" %s", get_date_string().c_str());
//
//     ImGui::Text("Memory");
//     ImGui::SameLine();
//     ImGui::ProgressBar(get_memory_usage());
//
// }

struct state {
    struct wl_display*    wl_display    = nullptr;
    struct wl_surface*    wl_surface    = nullptr;
    struct wl_registry*   wl_registry   = nullptr;
    struct wl_compositor* wl_compositor = nullptr;

    // xdg_wm_base*  xdg_wm_base  = nullptr;
    // xdg_surface*  xdg_surface  = nullptr;
    // xdg_toplevel* xdg_toplevel = nullptr;
    struct zwlr_layer_shell_v1* zwlr_layer_shell = nullptr;
    struct zwlr_layer_surface_v1* zwlr_layer_surface = nullptr;
    struct wl_egl_window* egl_window = nullptr;

    EGLDisplay egl_display = nullptr;
    EGLSurface egl_surface = nullptr;
    EGLContext egl_context = nullptr;
    EGLConfig  egl_config  = nullptr;
};

[[nodiscard]] bool init_egl(int width, int height, state& state) {

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

wl_registry_listener wl_registry_listener_ {
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

zwlr_layer_surface_v1_listener zwlr_layer_surface_v1_listener_ {
    .configure = [] (void* data, struct zwlr_layer_surface_v1* zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height) {
        zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);
        // state& state = *static_cast<struct state*>(data);
        // glViewport(0, 0, width, height);
        // wl_egl_window_resize(self.m_egl_window, width, height, 0, 0);
    },
    .closed = []([[maybe_unused]] void* data, [[maybe_unused]] struct zwlr_layer_surface_v1* zwlr_layer_surface_v1) { }
};

void draw() {
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

wl_callback_listener wl_callback_listener_ {
    .done = [] (void* data, struct wl_callback* wl_callback, [[maybe_unused]] uint32_t callback_data) {
        state& state = *static_cast<struct state*>(data);

        wl_callback_destroy(wl_callback);

        struct wl_callback* frame_callback = wl_surface_frame(state.wl_surface);
        wl_callback_add_listener(frame_callback, &wl_callback_listener_, &state);

        draw();
        eglSwapBuffers(state.egl_display, state.egl_surface);
    }
};

} // namespace

int main() {

    int width = 500;
    int height = 500;

    state state;

    state.wl_display = wl_display_connect(nullptr);
    state.wl_registry = wl_display_get_registry(state.wl_display);
    wl_registry_add_listener(state.wl_registry, &wl_registry_listener_, &state);
    wl_display_roundtrip(state.wl_display);

    state.wl_surface = wl_compositor_create_surface(state.wl_compositor);

    if (!init_egl(width, height, state))
        throw std::runtime_error("failed to initialize EGL");

    state.zwlr_layer_surface = zwlr_layer_shell_v1_get_layer_surface(state.zwlr_layer_shell, state.wl_surface, nullptr, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "wdock");

    zwlr_layer_surface_v1_add_listener(state.zwlr_layer_surface, &zwlr_layer_surface_v1_listener_, &state);
    zwlr_layer_surface_v1_set_size(state.zwlr_layer_surface, 500, 500);
    zwlr_layer_surface_v1_set_anchor(state.zwlr_layer_surface, ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    zwlr_layer_surface_v1_set_margin(state.zwlr_layer_surface, 0, 300, 0, 0);

    wl_surface_commit(state.wl_surface);
    wl_display_roundtrip(state.wl_display);

    wl_callback* frame_callback = wl_surface_frame(state.wl_surface);
    wl_callback_add_listener(frame_callback, &wl_callback_listener_, &state);

    eglSwapBuffers(state.egl_display, state.egl_surface);

    while (wl_display_dispatch(state.wl_display) != -1);

}

// int main2() {
//
//     int width = 1600;
//     int height = 900;
//
//     GLFWwindow* window = setup_glfw(width, height, "wdock");
//     imgui_init(window);
//     imgui_configure();
//
//     glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//
//     while (!glfwWindowShouldClose(window)) {
//         glClear(GL_COLOR_BUFFER_BIT);
//
//         imgui_set_next_window_dimensions(window);
//
//         with_imgui_frame_context([&] {
//
//             int flags = ImGuiWindowFlags_NoDecoration
//                 | ImGuiWindowFlags_NoMove;
//
//             ImGui::Begin("main", nullptr, flags);
//             imgui_draw();
//             ImGui::End();
//
//         });
//
//         if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//             break;
//
//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }
//
//     imgui_shutdown();
//
//     glfwDestroyWindow(window);
//     glfwTerminate();
//
// }