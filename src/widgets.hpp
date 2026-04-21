#pragma once

#include <chrono>
#include <print>
#include <filesystem>
#include <stdexcept>
#include <cstdio>

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <ctime>

#include <stb_image.h>

#include <glad/gl.h>

#include "imgui.h"

struct widget_error : std::runtime_error {
    using std::runtime_error::runtime_error;

    template <typename... Args>
    widget_error(std::format_string<Args...> fmt, Args... args)
    : widget_error(std::vformat(fmt.get(), std::make_format_args(args...)))
    { }
};

class widget {
    public:
    virtual ~widget() = default;
    virtual void draw() const = 0;
};

namespace widgets {

    struct style {
        std::string color_text;
        std::string color_button_active;
        std::string color_button;
        std::string color_button_hovered;
        std::string color_progress_fg;
        std::string color_progress_bg;
    };

    class custom : public widget {
        public:
        explicit custom(std::string command)
        : m_command(std::move(command))
        { }

        void draw() const override {
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
        image(const std::filesystem::path& path, float scaling)
        : m_scaling(scaling)
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

        void draw() const override {
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
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0, 1.0, 1.0, 1.0);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0, 0.0, 0.0, 1.0);

            ImGui::TextUnformatted(fmt.c_str());
            ImGui::SameLine();
            ImGui::ProgressBar(frac);

        }

    };

    class datetime : public widget {
        public:
        datetime(std::string timezone, std::string format)
        : m_timezone(std::move(timezone))
        , m_format(std::move(format))
        { }

        void draw() const override {
            auto& style = ImGui::GetStyle();
            style.Colors[ImGuiCol_Text] = ImVec4(0, 0, 1, 1);

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

    class kernel : public widget {
        public:
        kernel() = default;

        void draw() const override {
            struct utsname buf;
            assert(uname(&buf) == 0); auto fmt = std::format("{} {} {} {}", buf.sysname, buf.nodename, buf.release, buf.machine);
            ImGui::Text("%s", fmt.c_str());
        }

    };

    class button : public widget {
        public:
        button(std::string label, std::string on_click)
        : m_label(std::move(label))
        , m_on_click(std::move(on_click))
        { }

        void draw() const override {

            auto& style = ImGui::GetStyle();
            style.Colors[ImGuiCol_Button] = ImVec4(1.0, 0.0, 0.0, 1.0);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0, 0.0, 1.0, 1.0);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0, 1.0, 0.0, 1.0);

            if (ImGui::Button(m_label.c_str()))
                system(m_on_click.c_str());
        }

        private:
        const std::string m_label;
        const std::string m_on_click;

    };

} // namespace widgets