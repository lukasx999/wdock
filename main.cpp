#include <print>
#include <cassert>
#include <chrono>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

#include <cairomm/surface.h>
#include <cairomm/context.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_null.h>
#include <imgui_stdlib.h>

#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "wayland.hpp"

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

void imgui_init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void imgui_shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void with_imgui_frame_context(std::invocable auto fn, int width, int height) {
    ImGui_ImplOpenGL3_NewFrame();

    ImGui_ImplNullPlatform_NewFrame();

    // TODO:
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
    io.DeltaTime = 1.0f / 60.0f;

    // ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    fn();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imgui_configure() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.FontSizeBase = 40.0f;

    ImGuiIO& io = ImGui::GetIO();
    auto font_path = "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf";
    io.Fonts->AddFontFromFileTTF(font_path);
}

void imgui_set_next_window_dimensions() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize(io.DisplaySize);
}

void with_imgui_main_context(std::invocable auto fn) {
    int flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove;

    imgui_set_next_window_dimensions();
    ImGui::Begin("main", nullptr, flags);
    fn();
    ImGui::End();
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

    imgui_init();
    imgui_configure();

    glDebugMessageCallback(opengl_debug_message_callback, nullptr);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    surface.on_draw([&] {
        glClear(GL_COLOR_BUFFER_BIT);

        with_imgui_frame_context([&] {
            with_imgui_main_context(imgui_draw);
        }, surface.get_width(), surface.get_height());

    });

    surface.dispatch();
    imgui_shutdown();

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