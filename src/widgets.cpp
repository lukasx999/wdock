#include "widgets.hpp"

#include <fstream>

namespace widgets {

    void custom::on_draw() const {
        FILE* file = popen(m_command.c_str(), "r");
        if (file == nullptr)
            throw widget_error("error executing command: \"{}\"", m_command);

        std::string buf;

        char c;
        while ((c = fgetc(file)) != EOF)
            buf += c;

        // TODO: check for exit code
        if (not feof(file))
            throw widget_error("error reading output from command: \"{}\"", m_command);

        ImGui::TextUnformatted(buf.c_str());

        assert(pclose(file) != -1);
    }

    void disk::on_draw() const {
        struct statvfs buf;
        assert(statvfs("/", &buf) == 0);

        auto gibs = 1 / std::pow(2, 30);
        auto size = buf.f_frsize;

        uint64_t total = size * buf.f_blocks;
        uint64_t free = size * buf.f_bfree;
        uint64_t used = total - free;

        auto fmt = std::format(" {:.1f}GiB/{:.1f}GiB", used * gibs, total * gibs);

        ImGui::TextUnformatted(fmt.c_str());
        ImGui::SameLine();
        ImGui::ProgressBar(static_cast<float>(used) / total, {0, 0}, m_show_percentage ? nullptr : "");
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