#include "widgets.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace widgets {

    void custom::on_draw() const {
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

    image::image(widget_style style, const std::filesystem::path& path, float scaling)
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

    void disk::on_draw() const {
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
        ImGui::ProgressBar(static_cast<float>(used) / total, {0, 0}, m_show_percentage ? nullptr : "");
    }

    void player::on_draw() const {

        auto data = get_data();
        GError* err = nullptr;

        ImGui::Text("%s - %s - %s", data.artist, data.album, data.title);

        ImGui::TextUnformatted(std::format("{:%M}:{:%S}", data.position, std::chrono::duration_cast<std::chrono::seconds>(data.position)).c_str());
        ImGui::SameLine();

        ImGui::ProgressBar(static_cast<float>(data.position.count()) / data.length.count(), {0, 0}, "");

        ImGui::SameLine();
        ImGui::TextUnformatted(std::format("{:%M}:{:%S}", data.length, std::chrono::duration_cast<std::chrono::seconds>(data.length)).c_str());

        if (ImGui::Button("prev"))
            playerctl_player_previous(m_player, &err);

        auto play_text = data.status == PLAYERCTL_PLAYBACK_STATUS_PLAYING
            ? "pause"
            : "play";

        ImGui::SameLine();
        if (ImGui::Button(play_text))
            playerctl_player_play_pause(m_player, &err);

        ImGui::SameLine();
        if (ImGui::Button("󰼧"))
            playerctl_player_next(m_player, &err);

    }

} // namespace widgets