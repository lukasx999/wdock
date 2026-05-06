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

#include <stb_image.h>
#include <glad/gl.h>

#include "imgui.h"
#include "imgui_stdlib.h"
#include "utils.hpp"

struct widget_error : std::runtime_error {
    using std::runtime_error::runtime_error;

    template <typename... Args>
    widget_error(std::format_string<Args...> fmt, Args&&... args)
    : widget_error(std::format(fmt, std::forward<Args>(args)...))
    { }
};

struct widget_style {
    std::string color_frame_bg       = "#2e3440";
    std::string color_text           = "#eceff4";
    std::string color_button         = "#2e3440";
    std::string color_button_hovered = "#3b4252";
    std::string color_button_active  = "#434c5e";
    std::string color_progress       = "#4c566a";
    float frame_padding = 5;
    float frame_rounding = 5;
};

class widget {
    public:
    explicit widget(widget_style style)
    : m_style(style)
    { }

    virtual ~widget() = default;

    void draw() const {
        apply_style();
        on_draw();
    }

    protected:
    const widget_style m_style;

    virtual void on_draw() const { };

    void apply_style() const {
        auto& style = ImGui::GetStyle();

        style.FrameRounding = m_style.frame_rounding;
        style.FramePadding = ImVec2(m_style.frame_padding, m_style.frame_padding);

        set_color(ImGuiCol_Text, m_style.color_text);
        set_color(ImGuiCol_PlotHistogram, m_style.color_progress);
        set_color(ImGuiCol_FrameBg, m_style.color_frame_bg);
        set_color(ImGuiCol_Button, m_style.color_button);
        set_color(ImGuiCol_ButtonActive, m_style.color_button_active);
        set_color(ImGuiCol_ButtonHovered, m_style.color_button_hovered);
    }

    private:
    void set_color(ImGuiCol imgui_color, std::string_view color_string) const {

        auto color = parse_color_string(color_string);
        if (not color)
            throw widget_error("failed to parse color \"{}\"", color_string);

        auto& style = ImGui::GetStyle();
        style.Colors[imgui_color] = *color;

    }

};

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

    class image : public widget {
        public:
        image(widget_style style, const std::filesystem::path& path, float scaling);
        ~image();
        image(const image&) = delete;
        image(image&&) = delete;
        image& operator=(const image&) = delete;
        image& operator=(image&&) = delete;

        void on_draw() const override;

        private:
        const float m_scaling;
        int m_width;
        int m_height;
        GLuint m_texture_id = 0;

    };

    class memory : public widget {
        public:
        memory(widget_style style, bool show_percentage)
        : widget(style)
        , m_show_percentage(show_percentage)
        { }

        void on_draw() const override;

        private:
        const bool m_show_percentage;

        /// @brief parses a line from /proc/meminfo
        /// @returns the attribute, and the parsed value in KiB's
        [[nodiscard]] static auto parse_proc_meminfo_line(std::string_view line) -> std::tuple<std::string, uint64_t>;

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