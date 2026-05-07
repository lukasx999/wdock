#pragma once

#include "../widget.hpp"

namespace widgets {

    class disk : public widget {
        public:
        disk(widget_style style, bool show_percentage)
        : widget(style)
        , m_show_percentage(show_percentage)
        { }

        void on_draw() const override;

        private:
        const bool m_show_percentage;

    };

} // namespace widgets