#include <chrono>

#include "datetime.hpp"

namespace widgets {

    datetime::datetime(widget_style style, std::string timezone, std::string label, std::string format)
    : widget(style)
    , m_timezone(std::move(timezone))
    , m_label(std::move(label))
    , m_format(std::move(format))
    { }

    void datetime::on_draw() const {
        ImGui::TextUnformatted(m_label.c_str());
        ImGui::SameLine();
        auto time = get_formatted_time();
        ImGui::TextUnformatted(time.c_str());
    }

    std::string datetime::get_formatted_time() const {
        try {
            auto now = std::chrono::system_clock::now();
            std::chrono::zoned_time zt(m_timezone, now);

            time_t time = std::chrono::system_clock::to_time_t(zt);
            tm* tm = localtime(&time);

            std::stringstream fmt;
            fmt << std::put_time(tm, m_format.c_str());
            return fmt.str();

        } catch (const std::runtime_error&) {
            throw widget_error("invalid time zone: {}", m_timezone);
        }

    }

} // namespace widgets