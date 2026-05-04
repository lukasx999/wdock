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

    void load_config(const std::filesystem::path& config_path);

    void run() {
        m_window.run();
    }

    private:
    window m_window;
    ui m_ui;
    std::vector<std::unique_ptr<widget>> m_widgets;

    void draw() const;

};