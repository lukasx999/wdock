#pragma once

#include <chrono>
#include <string_view>

#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "imgui.h"

namespace widgets {

    class widget {
        public:
        virtual ~widget() = default;
        virtual void draw() const = 0;
    };

    class memory : public widget {
        public:
        memory() = default;

        void draw() const override {
            struct sysinfo buf{};
            assert(sysinfo(&buf) == 0);
            // TODO: get this right
            size_t total = buf.totalram / 1'000'000'000;
            size_t free = buf.freeram / 1'000'000'000;
            size_t used = total - free;
            auto fmt = std::format("{}Gib/{}Gib", used, total);
            float frac = static_cast<float>(used) / total;

            auto& style = ImGui::GetStyle();
            style.FrameRounding = 20;
            // style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0, 1.0, 1.0, 1.0);

            ImGui::TextUnformatted(fmt.c_str());
            ImGui::SameLine();
            ImGui::ProgressBar(frac);

        }

    };

    class date : public widget {
        public:
        explicit date(std::string_view timezone)
        : m_timezone(timezone)
        { }

        void draw() const override {
            auto now = std::chrono::system_clock::now();
            std::chrono::zoned_time zt(m_timezone, now);
            ImGui::Text(" %s", std::format("{:%d.%m.%Y}", zt).c_str());
        }

        private:
        const std::string_view m_timezone;

    };

    class time : public widget {
        public:
        explicit time(std::string_view timezone)
        : m_timezone(timezone)
        { }

        void draw() const override {
            auto now = std::chrono::system_clock::now();
            std::chrono::zoned_time zt(m_timezone, now);
            ImGui::Text(" %s", std::format("{:%H:%M}", zt).c_str());
        }

        private:
        const std::string_view m_timezone;

    };

    class kernel : public widget {
        public:
        kernel() = default;

        void draw() const override {
            struct utsname buf;
            assert(uname(&buf) == 0); auto fmt = std::format("{} {} {} {}", buf.sysname, buf.nodename, buf.release, buf.machine);
            ImGui::Text("%s", fmt.c_str());
        }

    };

} // namespace widgets