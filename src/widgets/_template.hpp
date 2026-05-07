#pragma once

#include "../widget.hpp"

namespace widgets {

    class new_widget : public widget {
        public:
        explicit new_widget(widget_style style)
        : widget(style)
        { }

        void on_draw() const override { }

    };

} // namespace widgets