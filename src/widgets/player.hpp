#pragma once

#include <playerctl/playerctl.h>

#include "../widget.hpp"

namespace widgets {

class player : public widget {
    public:
    /// @param player_name name of the music player, may be nullptr for automatic detection
    player(widget_style style, const char* player_name, bool show_album_art);

    ~player();
    player(const player&) = delete;
    player(player&&) noexcept = delete;
    player& operator=(const player&) = delete;
    player& operator=(player&&) noexcept = delete;

    void on_draw() const override;

    private:
    struct data {
        const char* title;
        const char* album;
        const char* artist;
        const char* art_url;
        std::chrono::microseconds length;
        std::chrono::microseconds position;
        PlayerctlPlaybackStatus status;
    };

    PlayerctlPlayer* m_player;
    const char* m_icon_pause = "";
    const char* m_icon_play  = "";
    const char* m_icon_next  = "";
    const char* m_icon_prev  = "";
    const float m_album_art_scaling = 0.25f;
    const bool m_show_album_art;

    void draw_album_art(const char* art_url) const;
    void draw_control_buttons(const data& data) const;
    void draw_progress_bar(const data& data) const;
    void draw_text(const data& data) const;
    [[nodiscard]] data get_data() const;

};

} // namespace widgets