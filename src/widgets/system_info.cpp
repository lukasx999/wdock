#include <format>
#include <chrono>

#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "system_info.hpp"

namespace widgets {

    void system_info::on_draw() const {

        struct sysinfo sysinfo_buf;
        if (sysinfo(&sysinfo_buf) != 0)
            throw widget_error("call to sysinfo() failed");

        std::chrono::seconds uptime(sysinfo_buf.uptime);

        struct utsname uname_buf;
        if (uname(&uname_buf) != 0)
            throw widget_error("call to uname() failed");

        auto fmt = std::format("{} {} {} {}", uname_buf.sysname, uname_buf.nodename, uname_buf.release, uname_buf.machine);

        if (!m_label.empty()) {
            ImGui::TextUnformatted(m_label.c_str());
            ImGui::SameLine();
        }
        ImGui::Text("%s", fmt.c_str());
        // ImGui::TextUnformatted(std::format("uptime: {}", uptime).c_str());
        // ImGui::Text("procs: %d", sysinfo_buf.procs);
    }


} // namespace widgets