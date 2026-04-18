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

    void load_config(const config& config) {
        auto& window = config.window;

        if (window.size)
            m_window.set_size(window.size->width, window.size->height);

        if (window.anchor)
            m_window.set_anchor(*window.anchor);

        if (window.margin)
            m_window.set_margin(*window.margin);

        if (not config.widgets.empty()) {
            m_widgets.clear();
            parse_widgets(config);
        }

    }

    void run() {
        m_window.run();
    }

    private:
    window m_window;
    ui m_ui;
    std::vector<std::unique_ptr<widgets::widget>> m_widgets;

    void draw() {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_ui.draw([&] {

            for (auto& widget : m_widgets) {
                widget->draw();
            }

        });

    }

    template <class Widget> requires std::is_base_of_v<widgets::widget, Widget>
    void add_widget(auto&&... args) {
        m_widgets.push_back(std::make_unique<Widget>(std::forward<decltype(args)>(args)...));
    }

    // TODO: add checking for invalid options
    void parse_widgets(const config& config) {

        for (auto& widget_name : config.widgets) {

            if (!config.widget_definitions.contains(widget_name))
                throw std::runtime_error(std::format("widget \"{}\" has not been defined.", widget_name));

            auto widget_def = config.widget_definitions.at(widget_name);
            auto preset = widget_def.preset;
            auto& props = widget_def.properties;

            if (preset == "datetime") {
                auto timezone = string_from_u8(props["timezone"].front().as<std::u8string>());
                auto format = string_from_u8(props["format"].front().as<std::u8string>());

                add_widget<widgets::datetime>(timezone, format);

            } else if (preset == "image") {
                auto path = string_from_u8(props["path"].front().as<std::u8string>());
                auto scaling = props["scaling"].front().as<float>();

                add_widget<widgets::image>(path, scaling);

            } else if (preset == "kernel") {
                add_widget<widgets::kernel>();

            } else if (preset == "button") {
                auto label = string_from_u8(props["label"].front().as<std::u8string>());
                auto on_click = string_from_u8(props["on_click"].front().as<std::u8string>());

                add_widget<widgets::button>(label, on_click);

            } else
                throw std::runtime_error(std::format("widget preset \"{}\" does not exist.", widget_def.preset));

        }

    }

};