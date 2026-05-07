#include <format>
#include <chrono>

#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "system_info.hpp"

namespace widgets {

    void system_info::on_draw() const {
        struct sysinfo sysinfo_buf;
        assert(sysinfo(&sysinfo_buf) == 0);
        std::chrono::seconds uptime(sysinfo_buf.uptime);

        struct utsname uname_buf;
        assert(uname(&uname_buf) == 0);
        auto fmt = std::format("{} {} {} {}", uname_buf.sysname, uname_buf.nodename, uname_buf.release, uname_buf.machine);

        ImGui::Text("%s", fmt.c_str());
        ImGui::TextUnformatted(std::format("uptime: {}", uptime).c_str());
        ImGui::Text("procs: %d", sysinfo_buf.procs);
    }


} // namespace widgets