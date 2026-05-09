#include <unordered_map>
#include <cstring>
#include <chrono>
#include <format>

#include "player.hpp"
#include "image.hpp"

namespace widgets {

    player::player(widget_style style, const char* player_name)
    : widget(style)
    {
        GError* err = nullptr;

        m_player = playerctl_player_new(player_name, &err);
        if (err != nullptr)
            throw widget_error("failed to create player: \"{}\"", err->message);
    }

    player::~player() {
        g_object_unref(m_player);
    }

    void player::draw_control_buttons(const data& data) const {
        GError* err = nullptr;

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

    }

    void player::on_draw() const {

        auto data = get_data();

        draw_album_art(data.art_url);

        ImGui::SameLine();
        ImGui::Text("%s - %s - %s", data.artist, data.album, data.title);

        ImGui::TextUnformatted(std::format("{:%M}:{:%S}", data.position, std::chrono::duration_cast<std::chrono::seconds>(data.position)).c_str());
        ImGui::SameLine();

        ImGui::ProgressBar(static_cast<float>(data.position.count()) / data.length.count(), {0, 0}, "");

        ImGui::SameLine();
        ImGui::TextUnformatted(std::format("{:%M}:{:%S}", data.length, std::chrono::duration_cast<std::chrono::seconds>(data.length)).c_str());

        draw_control_buttons(data);

    }

    void player::draw_album_art(const char* art_url) const {
        static std::unordered_map<std::string, std::filesystem::path> url_path_map;
        static uint64_t counter = 0;

        std::filesystem::create_directory("/tmp/wdock");
        std::string art_path;

        if (url_path_map.contains(art_url)) {
            art_path = url_path_map.at(art_url);

        } else {
            art_path = std::format("/tmp/wdock/album-art-{}", counter++);

            print_debug("downloading album art from \"{}\" to \"{}\"", art_url, art_path);
            if (not download_file(art_url, art_path))
                throw widget_error("failed to download album art from \"{}\"", art_url);

            url_path_map.insert({art_url, art_path});
        }

        image image(m_style, art_path, m_album_art_scaling);
        image.draw();

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