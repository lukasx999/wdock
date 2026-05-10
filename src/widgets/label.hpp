#pragma once

#include "../widget.hpp"

namespace widgets {

    class label : public widget {
        public:
        label(widget_style style, std::string text)
        : widget(style)
        , m_text(std::move(text))
        { }

        void on_draw() const override {
            ImGui::TextUnformatted(m_text.c_str());
        }

        private:
        const std::string m_text;

    };

} // namespace widgets