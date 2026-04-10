#include "imgui_internal.h"
#include <print>
#include <string_view>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#include <cairomm/surface.h>
#include <cairomm/context.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(
        window, []([[maybe_unused]] GLFWwindow* win, int w, int h) {
            glViewport(0, 0, w, h);
        }
    );

    #if 0
    glDebugMessageCallback([](
        [[maybe_unused]] GLenum src,
        [[maybe_unused]] GLenum type,
        [[maybe_unused]] GLuint id,
        [[maybe_unused]] GLenum severity,
        [[maybe_unused]] GLsizei len,
        const char* msg,
        [[maybe_unused]] const void* args
    ) { std::println(stderr, "opengl error: {}", msg); }, nullptr);
    #endif

    return window;
}

void imgui_init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void imgui_shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void with_imgui_frame_context(std::invocable auto fn) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    fn();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imgui_configure() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.FontSizeBase = 40.0f;

    ImGuiIO& io = ImGui::GetIO();
    auto font_path = "/usr/share/fonts/TTF/Roboto-Regular.ttf";
    io.Fonts->AddFontFromFileTTF(font_path);
}

void imgui_set_next_window_dimensions(GLFWwindow* window) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize(ImVec2(w, h));
}

void imgui_draw() {
    static bool is_checked = false;

    ImGui::Checkbox("check", &is_checked);
    if (is_checked)
        ImGui::Text("check!");

    ImGui::Text("hello, world");
    if (ImGui::Button("click me"))
        std::println("hello");
}

} // namespace

int main() {

    int width = 1600;
    int height = 900;

    GLFWwindow* window = setup_glfw(width, height, "wdock");
    imgui_init(window);
    imgui_configure();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        imgui_set_next_window_dimensions(window);

        with_imgui_frame_context([&] {

            int flags = ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_NoMove;

            ImGui::Begin("main", nullptr, flags);
            imgui_draw();
            ImGui::End();

        });

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    imgui_shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

}