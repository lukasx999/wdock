#pragma once

#include "../widget.hpp"

namespace widgets {

    class system_info : public widget {
        public:
        system_info(widget_style style, std::string label, std::string format)
        : widget(style)
        , m_label(std::move(label))
        , m_format(std::move(format))
        { }

        void on_draw() const override;

        private:
        struct data {
            std::string sysname;
            std::string nodename;
            std::string release;
            std::string machine;
            std::chrono::seconds uptime;
            int procs;
        };

        const std::string m_label;
        const std::string m_format;

        [[nodiscard]] static data get_data();

    };

} // namespace widgets