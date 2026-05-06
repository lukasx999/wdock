#pragma once

#include <print>
#include <cmath>
#include <cstring>
#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <cstdio>
#include <ctime>

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "imgui.h"
#include "imgui_stdlib.h"
#include "utils.hpp"

#include "widget.hpp"
#include "widgets/image.hpp"
#include "widgets/player.hpp"
#include "widgets/memory.hpp"

namespace widgets {

    class custom : public widget {
        public:
        custom(widget_style style, std::string command)
        : widget(style)
        , m_command(std::move(command))
        { }

        void on_draw() const override;

        private:
        const std::string m_command;

    };

    class disk : public widget {
        public:
        disk(widget_style style, bool show_percentage)
        : widget(style)
        , m_show_percentage(show_percentage)
        { }

        void on_draw() const override;

        private:
        const bool m_show_percentage;

    };

    class datetime : public widget {
        public:
        datetime(widget_style style, std::string timezone, std::string format)
        : widget(style)
        , m_timezone(std::move(timezone))
        , m_format(std::move(format))
        { }

        void on_draw() const override {
            auto time = get_formatted_time();
            ImGui::TextUnformatted(time.c_str());
        }

        private:
        const std::string m_timezone;
        const std::string m_format;

        [[nodiscard]] std::string get_formatted_time() const;

    };

    class system_info : public widget {
        public:
        explicit system_info(widget_style style)
        : widget(style)
        { }

        void on_draw() const override {
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

    };

    class button : public widget {
        public:
        button(widget_style style, std::string label, std::string on_click)
        : widget(style)
        , m_label(std::move(label))
        , m_on_click(std::move(on_click))
        { }

        void on_draw() const override {
            if (ImGui::Button(m_label.c_str()))
                system(m_on_click.c_str());
        }

        private:
        const std::string m_label;
        const std::string m_on_click;

    };


} // namespace widgets