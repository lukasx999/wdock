#pragma once

#include <print>

#include "window.hpp"
#include "ui.hpp"
#include "widgets.hpp"

class application {
    public:
    application(int width, int height)
    : m_window(width, height, "wdock", window::anchor::right, {0, 200, 0, 0})
    {
        glDebugMessageCallback(opengl_debug_message_callback, nullptr);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    }

    void run() {

        m_window.on_draw([&] {
            glClear(GL_COLOR_BUFFER_BIT);
            m_ui.draw(m_window.get_width(), m_window.get_height(), [&] {
                m_date.draw();
                m_time.draw();
                m_kernel.draw();
            });
        });

        m_window.run();

    }

    private:
    window m_window;
    ui m_ui;
    widgets::date m_date;
    widgets::time m_time;
    widgets::kernel m_kernel;

    static void opengl_debug_message_callback([[maybe_unused]] GLenum src, [[maybe_unused]] GLenum type, [[maybe_unused]] GLuint id, [[maybe_unused]] GLenum severity, [[maybe_unused]] GLsizei len, const char* msg, [[maybe_unused]] const void* args) {
        std::println(stderr, "opengl error: {}", msg);
    }

};