#pragma once

#include "../widget.hpp"

namespace widgets {

    class system_info : public widget {
        public:
        explicit system_info(widget_style style)
        : widget(style)
        { }

        void on_draw() const override;

    };

} // namespace widgets