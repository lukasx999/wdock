#pragma once

#include <mutex>
#include <filesystem>

#include "window.hpp"
#include "ui.hpp"
#include "widget.hpp"

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

    // this lock exists, so that we can make sure that the main thread is not
    // rendering to the window, while the config watcher thread tries to reload the config.
    mutable std::mutex m_draw_lock;


    void draw() const;

};