#pragma once

#include "window.hpp"
#include "ui.hpp"
#include "widgets.hpp"
#include "config.hpp"

class application {
    public:
    application(int width, int height)
    : m_window("wdock", width, height)
    , m_ui(m_window.get_wl_display(), m_window.get_wl_egl_window())
    {
        m_window.on_draw([&] {
            draw();
        });
    }

    void load_config(config& config) {
        load_window_config(config.window);

        if (not config.widgets.empty()) {
            m_widgets = std::move(config.widgets);
        }

    }

    void run() {
        m_window.run();
    }

    private:
    window m_window;
    ui m_ui;
    std::vector<std::unique_ptr<widget>> m_widgets;

    void draw() const {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_ui.draw([&] {
            for (auto& widget : m_widgets) {
                widget->draw();
            }
        });

    }

    void load_window_config(const struct config::window& window) {
        if (window.size)
            m_window.set_size(window.size->width, window.size->height);

        if (window.anchor)
            m_window.set_anchor(*window.anchor);

        if (window.layer)
            m_window.set_layer(*window.layer);

        if (window.margin)
            m_window.set_margin(*window.margin);
    }

};