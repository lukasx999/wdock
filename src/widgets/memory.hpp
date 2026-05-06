#pragma once

#include "../widget.hpp"

namespace widgets {

    class memory : public widget {
        public:
        memory(widget_style style, bool show_percentage)
        : widget(style)
        , m_show_percentage(show_percentage)
        { }

        void on_draw() const override;

        private:
        const bool m_show_percentage;

        /// @brief parses a line from /proc/meminfo
        /// @returns the attribute, and the parsed value in KiB's
        [[nodiscard]] static auto parse_proc_meminfo_line(std::string_view line) -> std::tuple<std::string, uint64_t>;

    };

} // namespace widgets