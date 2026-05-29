#include <cmath>

#include <sys/statvfs.h>

#include "disk.hpp"

namespace widgets {

    void disk::on_draw() const {
        struct statvfs buf;
        if (statvfs("/", &buf) != 0)
            throw widget_error("failed to statvfs on \"/\"");

        auto gibs = 1 / std::pow(2, 30);
        auto size = buf.f_frsize;

        uint64_t total = size * buf.f_blocks;
        uint64_t free = size * buf.f_bfree;
        uint64_t used = total - free;

        auto fmt = std::format("{:.1f}GiB/{:.1f}GiB", used * gibs, total * gibs);

        if (!m_label.empty()) {
            ImGui::TextUnformatted(m_label.c_str());
            ImGui::SameLine();
        }

        ImGui::TextUnformatted(fmt.c_str());
        ImGui::SameLine();
        ImGui::ProgressBar(static_cast<float>(used) / total, {300, 32}, m_show_percentage ? nullptr : "");
    }

} // namespace widgets