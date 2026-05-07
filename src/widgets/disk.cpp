#include <cmath>

#include <sys/statvfs.h>

#include "disk.hpp"

namespace widgets {

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

} // namespace widgets