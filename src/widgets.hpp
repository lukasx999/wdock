#pragma once

#include <chrono>

#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "imgui.h"

namespace widgets {

    class widget {
        public:
        virtual ~widget() = default;
        virtual void draw() const = 0;
    };

    class date : public widget {
        public:
        date() = default;

        void draw() const override {
            auto now = std::chrono::system_clock::now();
            std::chrono::zoned_time zt("Europe/Vienna", now);
            ImGui::Text(" %s", std::format("{:%d.%m.%Y}", zt).c_str());
        }

    };

    class time : public widget {
        public:
        time() = default;

        void draw() const override {
            auto now = std::chrono::system_clock::now();
            std::chrono::zoned_time zt("Europe/Vienna", now);
            ImGui::Text(" %s", std::format("{:%H:%M}", zt).c_str());
        }

    };

    class kernel : public widget {
        public:
        kernel() = default;

        void draw() const override {
            struct utsname buf;
            assert(uname(&buf) == 0);
            auto fmt = std::format("{} {} {} {}", buf.sysname, buf.nodename, buf.release, buf.machine);
            ImGui::Text("%s", fmt.c_str());
        }

    };

} // namespace widgets