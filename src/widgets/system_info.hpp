#pragma once

#include "../widget.hpp"

namespace widgets {

    class system_info : public widget {
        public:
        system_info(widget_style style, std::string label)
        : widget(style)
        , m_label(std::move(label))
        { }

        void on_draw() const override;

        private:
        const std::string m_label;

    };

} // namespace widgets