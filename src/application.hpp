#pragma once

#include "window.hpp"
#include "ui.hpp"
#include "widgets.hpp"

class application {
    public:
    using anchor = window::anchor;
    using margin = window::margin;
    using widgets = std::vector<std::unique_ptr<widgets::widget>>;

    application(int width, int height, anchor anchor, margin margin, widgets widgets)
    : m_window("wdock", width, height, anchor, margin)
    , m_ui(m_window.get_wl_display(), m_window.get_wl_egl_window())
    , m_widgets(std::move(widgets))
    {
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

        m_ui.draw([&] {
            for (auto& widget : m_widgets) {
                widget->draw();
            }
        });
    }

};