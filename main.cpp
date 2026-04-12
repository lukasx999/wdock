#include <print>
#include <cassert>
#include <chrono>
#include <thread>

#include <cairomm/surface.h>
#include <cairomm/context.h>

#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "imgui.h"
#include "window.hpp"
#include "ui.hpp"

namespace {

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
        static int counter = 0;
        static auto time = std::chrono::steady_clock::now();

        if (auto now = std::chrono::steady_clock::now(); now >= time) {
            using namespace std::chrono_literals;
            counter++;
            time = now + 10ms;
        }

        ImGui::Text("welcome to wdock");
        ImGui::Text("%s", get_uname_string().c_str());
        ImGui::Text(" %s", get_time_string().c_str());
        ImGui::Text(" %s", get_date_string().c_str());

        ImGui::Text("Memory");
        ImGui::SameLine();
        ImGui::ProgressBar(get_memory_usage());

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 20));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15);
        ImGui::Button("click me");
        ImGui::PopStyleVar(2);

        ImGui::Button("<");
        ImGui::SameLine();
        ImGui::Button("||");
        ImGui::SameLine();
        ImGui::Button(">");

        ImGui::Text("counter: %d", counter);

    }

    void opengl_debug_message_callback([[maybe_unused]] GLenum src, [[maybe_unused]] GLenum type, [[maybe_unused]] GLuint id, [[maybe_unused]] GLenum severity, [[maybe_unused]] GLsizei len, const char* msg, [[maybe_unused]] const void* args) {
        std::println(stderr, "opengl error: {}", msg);
    }

} // namespace

int main_() {

    int width = 700;
    int height = 800;
    const char* title = "wdock";

    window window(width, height, title, window::anchor::right, {0, 200, 0, 0});
    ui ui;

    glDebugMessageCallback(opengl_debug_message_callback, nullptr);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    window.on_draw([&] {
        glClear(GL_COLOR_BUFFER_BIT);
        ui.draw(window.get_width(), window.get_height(), imgui_draw);
    });

    window.run();
}

int main2() {

    window a(500, 500, "a", window::anchor::left);
    window b(500, 500, "b", window::anchor::right);

    a.on_draw([&] {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    });

    b.on_draw([&] {
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    });

    std::vector<const window*> windows{ &a, &b };

    window::run_concurrent(windows);

    return 0;

}

int main() {

    window a(500, 500, "a", window::anchor::left);
    a.on_draw([&] {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    });
    window b(std::move(a));
    b.run();

    return 0;

}