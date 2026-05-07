#include <chrono>

#include "datetime.hpp"

namespace widgets {

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