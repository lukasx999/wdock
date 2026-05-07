#pragma once

#include "window.hpp"
#include "ui.hpp"

class application {
    public:
    application();

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