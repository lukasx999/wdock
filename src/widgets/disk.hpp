#pragma once

#include "../widget.hpp"

namespace widgets {

    class disk : public widget {
        public:
        disk(widget_style style, std::string label, bool show_percentage)
        : widget(style)
        , m_label(std::move(label))
        , m_show_percentage(show_percentage)
        { }

        void on_draw() const override;

        private:
        const std::string m_label;
        const bool m_show_percentage;

    };

} // namespace widgets