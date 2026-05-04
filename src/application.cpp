#include "application.hpp"

void application::load_config(const std::filesystem::path& config_path) {

    auto config = parse_config(config_path);

    auto& window = config.window;
    m_window.set_size(window.size.width, window.size.height);
    m_window.set_anchor(window.anchor);
    m_window.set_layer(window.layer);
    m_window.set_margin(window.margin);

    m_ui.load_style(config.window.style);

    if (not config.widgets.empty())
        m_widgets = std::move(config.widgets);

}

void application::draw() const {
    std::scoped_lock lock(g_draw_lock);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_ui.draw([&] {
        for (auto& widget : m_widgets) {
            widget->draw();
        }
    });

}