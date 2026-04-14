#pragma once

#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <string_view>

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <ctime>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/gl.h>

#include "imgui.h"

namespace widgets {

    struct style {

    };

    struct widget_error : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class widget {
        public:
        virtual ~widget() = default;
        virtual void draw() const = 0;
    };

    class custom : public widget {
        public:
        explicit custom(std::string_view command)
        : m_command(command)
        { }

        void draw() const override {
            // TODO: run the shell command and display its output
        }

        private:
        const std::string_view m_command;

    };

    // TODO: add scaling option
    class image : public widget {
        public:
        explicit image(const std::filesystem::path& path) {
            int channels;

            unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 0);
            if (data == nullptr)
                throw widget_error(std::format("failed to load image at {}", path.c_str()));

            GLenum format = [&] {
                switch (channels) {
                    case 3: return GL_RGB;
                    case 4: return GL_RGBA;
                    default:
                    stbi_image_free(data);
                    throw widget_error(std::format("invalid amount of channels ({})", channels));
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
            ImGui::Image(m_texture_id, ImVec2(m_width, m_height));
        }

        private:
        int m_width = 0;
        int m_height = 0;
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
            // style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0, 1.0, 1.0, 1.0);

            ImGui::TextUnformatted(fmt.c_str());
            ImGui::SameLine();
            ImGui::ProgressBar(frac);

        }

    };

    class datetime : public widget {
        public:
        datetime(std::string_view timezone, const char* format)
        : m_timezone(timezone)
        , m_format(format)
        { }

        void draw() const override {
            auto now = std::chrono::system_clock::now();
            try {
                std::chrono::zoned_time zt(m_timezone, now);

                time_t time = std::chrono::system_clock::to_time_t(zt);
                tm* tm = localtime(&time);

                std::stringstream fmt_stream;
                fmt_stream << std::put_time(tm, m_format);

                ImGui::TextUnformatted(fmt_stream.str().c_str());

            } catch (const std::runtime_error& error) {
                throw widget_error(std::format("invalid time zone: {}", m_timezone));
            }
        }

        private:
        const std::string_view m_timezone;
        const char* m_format;

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