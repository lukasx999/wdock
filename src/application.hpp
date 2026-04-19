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
        load_window_config(config.window);

        if (not config.widgets.empty()) {
            m_widgets.clear();
            load_widgets(config);
        }

    }

    void run() {
        m_window.run();
    }

    private:
    using widget_properties = config::widget_definition::properties;

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

        });

    }

    template <class Widget> requires std::is_base_of_v<widgets::widget, Widget>
    void add_widget(auto&&... args) {
        m_widgets.push_back(std::make_unique<Widget>(std::forward<decltype(args)>(args)...));
    }

    // TODO: parse widgets in config.hpp?
    void parse_widget_datetime(const widget_properties& props) {
        std::string timezone = "Europe/Vienna";
        std::string format = "%d.%m.%Y";

        for (auto& [name, values] : props) {

            if (name == "timezone")
                timezone = string_from_u8(values.front().as<std::u8string>());

            else if (name == "format")
                format = string_from_u8(values.front().as<std::u8string>());

            else
                throw config_error(std::format("property \"{}\" does not exist in widget \"datetime\".", name));
        }

        add_widget<widgets::datetime>(timezone, format);

    }

    // TODO: add checking for invalid options
    void load_widgets(const config& config) {

        for (auto& widget_name : config.widgets) {

            if (!config.widget_definitions.contains(widget_name))
                throw config_error(std::format("widget \"{}\" has not been defined.", widget_name));

            auto widget_def = config.widget_definitions.at(widget_name);
            auto preset = widget_def.preset;
            auto& props = widget_def.props;

            if (preset == "datetime") {
                parse_widget_datetime(props);

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