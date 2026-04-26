#pragma once

#include <print>
#include <cmath>
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

struct widget_error : std::runtime_error {
    using std::runtime_error::runtime_error;

    template <typename... Args>
    widget_error(std::format_string<Args...> fmt, Args... args)
    : widget_error(std::vformat(fmt.get(), std::make_format_args(args...)))
    { }
};

struct widget_style {
    std::string color_text;
    std::string color_button_active;
    std::string color_button;
    std::string color_button_hovered;
    std::string color_progress_fg;
    std::string color_progress_bg;
    float frame_padding = 0;
    float frame_rounding = 0;
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
    virtual void on_draw() const { };

    void apply_style() const {
        auto& style = ImGui::GetStyle();

        style.FrameRounding = m_style.frame_rounding;
        style.FramePadding = ImVec2(m_style.frame_padding, m_style.frame_padding);
        // style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0, 1.0, 1.0, 1.0);
        // style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0, 0.0, 0.0, 1.0);
        // style.Colors[ImGuiCol_Text] = ImVec4(0, 0, 1, 1);
        // style.Colors[ImGuiCol_Button] = ImVec4(1.0, 0.0, 0.0, 1.0);
        // style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0, 0.0, 1.0, 1.0);
        // style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0, 1.0, 0.0, 1.0);
    }

    private:
    const widget_style m_style;

};

namespace widgets {


    class custom : public widget {
        public:
        custom(widget_style style, std::string command)
        : widget(style)
        , m_command(std::move(command))
        { }

        void on_draw() const override {
            FILE* file = popen(m_command.c_str(), "r");
            if (file == nullptr)
                throw widget_error("error executing command: \"{}\"", m_command);

            std::string buf;

            char c;
            while ((c = fgetc(file)) != EOF) {
                buf += c;
            }

            // TODO: check for exit code
            if (not feof(file))
                throw widget_error("error reading output from command: \"{}\"", m_command);

            ImGui::TextUnformatted(buf.c_str());

            assert(pclose(file) != -1);
        }

        private:
        const std::string m_command;

    };

    class image : public widget {
        public:
        image(widget_style style, const std::filesystem::path& path, float scaling)
        : widget(style)
        , m_scaling(scaling)
        {
            int channels;
            unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 0);
            if (data == nullptr)
                throw widget_error("failed to load image at {}", path.c_str());

            GLenum format = [&] {
                switch (channels) {
                    case 3: return GL_RGB;
                    case 4: return GL_RGBA;
                    default:
                    stbi_image_free(data);
                    throw widget_error("invalid amount of channels ({})", channels);
                }
            }();

            glGenTextures(1, &m_texture_id);
            glBindTexture(GL_TEXTURE_2D, m_texture_id);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        ~image() {
            glDeleteTextures(1, &m_texture_id);
        }

        image(const image&) = delete;
        image(image&&) = delete;
        image& operator=(const image&) = delete;
        image& operator=(image&&) = delete;

        void on_draw() const override {
            ImVec2 size(m_width * m_scaling, m_height * m_scaling);
            ImGui::Image(m_texture_id, size);
        }

        private:
        const float m_scaling;
        int m_width;
        int m_height;
        GLuint m_texture_id = 0;

    };

    class memory : public widget {
        public:
        explicit memory(widget_style style)
        : widget(style)
        { }

        void on_draw() const override {
            struct sysinfo buf{};
            assert(sysinfo(&buf) == 0);
            // TODO: get this right
            size_t total = buf.totalram / 1'000'000'000;
            size_t free = buf.freeram / 1'000'000'000;
            size_t used = total - free;
            auto fmt = std::format("{}Gib/{}Gib", used, total);
            float frac = static_cast<float>(used) / total;

            ImGui::TextUnformatted(fmt.c_str());
            ImGui::SameLine();
            ImGui::ProgressBar(frac);

        }

    };

    class disk : public widget {
        public:
        explicit disk(widget_style style)
        : widget(style)
        { }

        void on_draw() const override {
            struct statvfs buf;
            assert(statvfs("/", &buf) == 0);

            auto gibs = 1 / std::pow(2, 30);
            auto size = buf.f_frsize;

            int total = size * buf.f_blocks * gibs;
            // int available = size * buf.f_bavail * gibs;
            int free = size * buf.f_bfree * gibs;
            int used = total - free;

            ImGui::Text("%dGiB/%dGiB", used, total);
            ImGui::SameLine();
            ImGui::ProgressBar(static_cast<float>(used) / total);
        }

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

        [[nodiscard]] std::string get_formatted_time() const {
            try {
                auto now = std::chrono::system_clock::now();
                std::chrono::zoned_time zt(m_timezone, now);

                time_t time = std::chrono::system_clock::to_time_t(zt);
                tm* tm = localtime(&time);

                std::stringstream fmt;
                fmt << std::put_time(tm, m_format.c_str());
                return fmt.str();

            } catch (const std::runtime_error& error) {
                throw widget_error("invalid time zone: {}", m_timezone);
            }

        }

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