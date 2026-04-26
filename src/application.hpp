#pragma once

#include "window.hpp"
#include "ui.hpp"
#include "widgets.hpp"
#include "config.hpp"

class application {
    public:
    application()
    : m_window("wdock", 100, 100)
    , m_ui(m_window.get_wl_display(), m_window.get_wl_egl_window())
    {
        m_window.on_draw([&] {
            draw();
        });
    }

    void load_config(config& config) {

        auto& window = config.window;
        m_window.set_size(window.size.width, window.size.height);
        m_window.set_anchor(window.anchor);
        m_window.set_layer(window.layer);
        m_window.set_margin(window.margin);

        m_ui.load_style(config.window.style);

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
        std::scoped_lock lock(g_application_lock);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_ui.draw([&] {
            for (auto& widget : m_widgets) {
                widget->draw();
            }
        });

    }

};