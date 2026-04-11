#include <print>
#include <cassert>
#include <chrono>

#define USE_GLFW

#ifdef USE_GLFW
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#endif // USE_GLFW

#include <cairomm/surface.h>
#include <cairomm/context.h>


#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "wayland.hpp"
#include "ui.hpp"

namespace {

[[nodiscard]] GLFWwindow* setup_glfw(int width, int height, const char* window_title) {
    glfwSetErrorCallback([]([[maybe_unused]] int error_code, char const* desc) {
        std::println(stderr, "glfw error: {}", desc);
    });

    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    GLFWwindow* window = glfwCreateWindow(width, height, window_title, nullptr, nullptr);

    glfwMakeContextCurrent(window);
    // gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(
        window, []([[maybe_unused]] GLFWwindow* win, int w, int h) {
            glViewport(0, 0, w, h);
        }
    );


    return window;
}

void opengl_debug_message_callback([[maybe_unused]] GLenum src, [[maybe_unused]] GLenum type, [[maybe_unused]] GLuint id, [[maybe_unused]] GLenum severity, [[maybe_unused]] GLsizei len, const char* msg, [[maybe_unused]] const void* args) {
    std::println(stderr, "opengl error: {}", msg);
}

[[nodiscard]] std::string get_time_string() {
    auto now = std::chrono::system_clock::now();
    std::chrono::zoned_time zt("Europe/Vienna", now);
    return std::format("{:%H:%M}", zt);
}

[[nodiscard]] std::string get_date_string() {
    auto now = std::chrono::system_clock::now();
    std::chrono::zoned_time zt("Europe/Vienna", now);
    return std::format("{:%d.%m.%Y}", zt);
}

[[nodiscard]] std::string get_uname_string() {
    struct utsname buf;
    assert(uname(&buf) == 0);
    return std::format("{} {} {} {}", buf.sysname, buf.nodename, buf.release, buf.machine);
}

[[nodiscard]] float get_memory_usage() {
    struct sysinfo buf;
    assert(sysinfo(&buf) == 0);
    return static_cast<float>(buf.totalram - buf.freeram) / buf.totalram;
}

void imgui_draw() {

    ImGui::Text("welcome to wdock");
    ImGui::Text("%s", get_uname_string().c_str());
    ImGui::Text(" %s", get_time_string().c_str());
    ImGui::Text(" %s", get_date_string().c_str());

    ImGui::Text("Memory");
    ImGui::SameLine();
    ImGui::ProgressBar(get_memory_usage());

}

} // namespace

int main() {

    wayland_layer_surface surface(500, 500, "wdock", wayland_layer_surface::anchor::top);
    ui ui;

    glDebugMessageCallback(opengl_debug_message_callback, nullptr);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    surface.on_draw([&] {
        glClear(GL_COLOR_BUFFER_BIT);
        ui.draw(surface.get_width(), surface.get_height(), imgui_draw);
    });

    surface.dispatch();

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