#pragma once

#include <print>

#include "window.hpp"
#include "ui.hpp"
#include "widgets.hpp"

class application {
    public:
    using anchor = window::anchor;
    using margin = window::margin;
    using widgets = std::vector<std::unique_ptr<widgets::widget>>;

    application(int width, int height, anchor anchor, margin margin, widgets widgets)
    : m_window(width, height, "wdock", anchor, margin)
    , m_widgets(std::move(widgets))
    {
        glDebugMessageCallback(opengl_debug_message_callback, nullptr);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        m_window.on_draw([&] {
            draw();
        });
    }

    void run() {
        m_window.run();
    }

    private:
    window m_window;
    ui m_ui;
    const widgets m_widgets;

    void draw() const {
        glClear(GL_COLOR_BUFFER_BIT);

        m_ui.draw(m_window.get_width(), m_window.get_height(), [&] {
            for (auto& widget : m_widgets) {
                widget->draw();
            }
        });
    }

    static void opengl_debug_message_callback([[maybe_unused]] GLenum src, [[maybe_unused]] GLenum type, [[maybe_unused]] GLuint id, [[maybe_unused]] GLenum severity, [[maybe_unused]] GLsizei len, const char* msg, [[maybe_unused]] const void* args) {
        std::println(stderr, "opengl error: {}", msg);
    }

};