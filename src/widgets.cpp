#include "widgets.hpp"

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace widgets {

    void custom::on_draw() const {
        FILE* file = popen(m_command.c_str(), "r");
        if (file == nullptr)
            throw widget_error("error executing command: \"{}\"", m_command);

        std::string buf;

        char c;
        while ((c = fgetc(file)) != EOF)
            buf += c;

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

    image::~image() {
        glDeleteTextures(1, &m_texture_id);
    }

    void image::on_draw() const {
        ImVec2 size(m_width * m_scaling, m_height * m_scaling);
        ImGui::Image(m_texture_id, size);
    }

    void memory::on_draw() const {
        std::ifstream file("/proc/meminfo");
        std::string line;

        int64_t total = -1;
        int64_t avail = -1;

        while (std::getline(file, line)) {
            auto [attribute, value] = parse_proc_meminfo_line(line);

            if (attribute == "MemTotal")
                total = value;

            else if (attribute == "MemAvailable")
                avail = value;

        }

        assert(total != -1);
        assert(avail != -1);

        int64_t used = total - avail;

        auto gibs = 1 / std::pow(2, 20);
        auto fmt = std::format("{:.1f}Gib/{:.1f}Gib", used * gibs, total * gibs);
        float frac = static_cast<float>(used) / total;

        ImGui::TextUnformatted(fmt.c_str());
        ImGui::SameLine();
        ImGui::ProgressBar(frac, {0, 0}, m_show_percentage ? nullptr : "");

    }

    auto memory::parse_proc_meminfo_line(std::string_view line) -> std::tuple<std::string, uint64_t> {

        size_t colon_pos = line.find(':');
        assert(colon_pos != std::string::npos);
        std::string attribute(line.substr(0, colon_pos));

        size_t value_start = line.find_first_not_of(' ', colon_pos+1);
        assert(value_start != std::string::npos);

        size_t value_end = line.find_first_of(' ', value_start);

        // some entries dont have a "kB" at the end, in that case the value
        // ends at the end of the line.
        size_t n = value_end == std::string::npos
            ? std::string::npos
            : value_end - value_start;

        auto value_string = line.substr(value_start, n);
        uint64_t value;

        auto ec = std::from_chars(value_string.data(), value_string.data() + value_string.size(), value).ec;
        assert(ec == std::errc{});

        return std::make_tuple(attribute, value);
    }

    void disk::on_draw() const {
        struct statvfs buf;
        assert(statvfs("/", &buf) == 0);

        auto gibs = 1 / std::pow(2, 30);
        auto size = buf.f_frsize;

        uint64_t total = size * buf.f_blocks * gibs;
        uint64_t free = size * buf.f_bfree * gibs;
        uint64_t used = total - free;

        auto fmt = std::format(" {} GiB / {} GiB", used, total);

        ImGui::TextUnformatted(fmt.c_str());
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

        if (ImGui::Button(m_icon_prev))
            playerctl_player_previous(m_player, &err);

        auto play_text = data.status == PLAYERCTL_PLAYBACK_STATUS_PLAYING
            ? m_icon_pause
            : m_icon_play;

        ImGui::SameLine();
        if (ImGui::Button(play_text))
            playerctl_player_play_pause(m_player, &err);

        ImGui::SameLine();
        if (ImGui::Button(m_icon_next))
            playerctl_player_next(m_player, &err);

        // TODO: fix messed up image
        // TODO: cache image for performance
        // auto art_path = "/tmp/wdock_art.png";
        // if (not download_file(data.art_url, art_path))
        //     throw widget_error("failed to download album art from \"{}\"", data.art_url);
        // image image(m_style, art_path, 0.5f);
        // image.draw();

    }

    player::data player::get_data() const {

        data data;
        GError* err = nullptr;

        data.title = playerctl_player_get_title(m_player, &err);
        if (err != nullptr) {
            data.title = "N/A";
            g_clear_error(&err);
        }

        data.album = playerctl_player_get_album(m_player, &err);
        if (err != nullptr) {
            data.album = "N/A";
            g_clear_error(&err);
        }

        data.artist = playerctl_player_get_artist(m_player, &err);
        if (err != nullptr) {
            data.artist = "N/A";
            g_clear_error(&err);
        }

        data.art_url = playerctl_player_print_metadata_prop(m_player, "mpris:artUrl", &err);
        if (err != nullptr) {
            data.art_url = nullptr;
            g_clear_error(&err);
        }

        g_main_context_iteration(nullptr, false);
        g_object_get(m_player, "playback-status", &data.status, nullptr);

        int64_t position_µs = playerctl_player_get_position(m_player, &err);
        if (err != nullptr) {
            position_µs = 0;
            g_clear_error(&err);
        }

        const char* length_str = playerctl_player_print_metadata_prop(m_player, "mpris:length", &err);
        if (err != nullptr) {
            length_str = "0";
            g_clear_error(&err);
        }

        int64_t length_µs;
        std::from_chars(length_str, length_str + std::strlen(length_str), length_µs);

        data.length = std::chrono::microseconds(length_µs);
        data.position = std::chrono::microseconds(position_µs);

        return data;
    }

} // namespace widgets