#pragma once

#include "../widget.hpp"

namespace widgets {

    class custom : public widget {
        public:
        custom(widget_style style, std::string label, std::string command)
        : widget(style)
        , m_label(std::move(label))
        , m_command(std::move(command))
        { }

        void on_draw() const override;

        private:
        const std::string m_label;
        const std::string m_command;

    };

} // namespace widgets