#pragma once

#include "window.hpp"
#include "ui.hpp"
#include "widgets.hpp"

class application {
    public:
    using anchor = window::anchor;
    using margin = window::margin;

    application(int width, int height, anchor anchor, margin margin)
    : m_window("wdock", width, height, anchor, margin)
    , m_ui(m_window.get_wl_display(), m_window.get_wl_egl_window())
    {
        m_window.on_draw([&] {
            draw();
        });
    }

    template <class Widget> requires std::is_base_of_v<widgets::widget, Widget>
    void add_widget(auto&&... args) {
        m_widgets.push_back(std::make_unique<Widget>(std::forward<decltype(args)>(args)...));
    }

    void run() {
        m_window.run();
    }

    private:
    window m_window;
    ui m_ui;
    std::vector<std::unique_ptr<widgets::widget>> m_widgets;

    void draw() const {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_ui.draw([&] {
            for (auto& widget : m_widgets) {
                widget->draw();
            }
            ImGui::Button("hello");
        });

    }

};