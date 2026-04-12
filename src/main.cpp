#include <print>
#include <cassert>
#include <chrono>

#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "imgui.h"
#include "window.hpp"
#include "ui.hpp"

namespace {

    class widget {
        public:
        virtual ~widget() = default;
        virtual void draw() const = 0;
    };

    class date_widget : public widget {
        public:
        date_widget() = default;

        void draw() const override {
            auto now = std::chrono::system_clock::now();
            std::chrono::zoned_time zt("Europe/Vienna", now);
            ImGui::Text(" %s", std::format("{:%d.%m.%Y}", zt).c_str());
        }

    };

    class time_widget : public widget {
        public:
        time_widget() = default;

        void draw() const override {
            auto now = std::chrono::system_clock::now();
            std::chrono::zoned_time zt("Europe/Vienna", now);
            ImGui::Text(" %s", std::format("{:%H:%M}", zt).c_str());
        }

    };

    class uname_widget : public widget {
        public:
        uname_widget() = default;

        void draw() const override {
            struct utsname buf;
            assert(uname(&buf) == 0);
            auto fmt = std::format("{} {} {} {}", buf.sysname, buf.nodename, buf.release, buf.machine);
            ImGui::Text("%s", fmt.c_str());
        }

    };

    [[nodiscard]] float get_memory_usage() {
        struct sysinfo buf;
        assert(sysinfo(&buf) == 0);
        return static_cast<float>(buf.totalram - buf.freeram) / buf.totalram;
    }

    void opengl_debug_message_callback([[maybe_unused]] GLenum src, [[maybe_unused]] GLenum type, [[maybe_unused]] GLuint id, [[maybe_unused]] GLenum severity, [[maybe_unused]] GLsizei len, const char* msg, [[maybe_unused]] const void* args) {
        std::println(stderr, "opengl error: {}", msg);
    }

    class application {
        public:
        application() = default;

        private:

    };

} // namespace

int main() {

    int width = 700;
    int height = 800;

    window window(width, height, "wdock", window::anchor::right, {0, 200, 0, 0});
    ui ui;

    date_widget date;
    time_widget time;
    uname_widget uname;

    glDebugMessageCallback(opengl_debug_message_callback, nullptr);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    window.on_draw([&] {
        glClear(GL_COLOR_BUFFER_BIT);
        ui.draw(window.get_width(), window.get_height(), [&] {
            date.draw();
            time.draw();
            uname.draw();

            ImGui::Text("welcome to wdock");

            ImGui::Text("Memory");
            ImGui::SameLine();
            ImGui::ProgressBar(get_memory_usage());

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.3, 0.3, 1.0)),
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 20));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15);
            ImGui::Button("click me");
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();

            ImGui::Button("<");
            ImGui::SameLine();
            ImGui::Button("||");
            ImGui::SameLine();
            ImGui::Button(">");
        });
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