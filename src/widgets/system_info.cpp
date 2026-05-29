#include <format>
#include <chrono>

#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "system_info.hpp"

namespace widgets {

    void system_info::on_draw() const {

        std::string fmt = m_format;
        auto data = get_data();

        replace_substring(fmt, "{sysname}", data.sysname);
        replace_substring(fmt, "{nodename}", data.nodename);
        replace_substring(fmt, "{sysname}", data.sysname);
        replace_substring(fmt, "{release}", data.release);
        replace_substring(fmt, "{machine}", data.machine);
        replace_substring(fmt, "{uptime}", std::format("{}", data.uptime));
        replace_substring(fmt, "{procs}", std::to_string(data.procs));

        if (!m_label.empty()) {
            ImGui::TextUnformatted(m_label.c_str());
            ImGui::SameLine();
        }
        ImGui::Text("%s", fmt.c_str());
    }

    system_info::data system_info::get_data() {

        struct sysinfo sysinfo_buf;
        if (sysinfo(&sysinfo_buf) != 0)
            throw widget_error("call to sysinfo() failed");

        struct utsname uname_buf;
        if (uname(&uname_buf) != 0)
            throw widget_error("call to uname() failed");

        return {
            uname_buf.sysname,
            uname_buf.nodename,
            uname_buf.release,
            uname_buf.machine,
            std::chrono::seconds(sysinfo_buf.uptime),
            sysinfo_buf.procs
        };
    }

} // namespace widgets