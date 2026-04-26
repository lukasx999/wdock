#pragma once

#include <functional>
#include <print>
#include <utility>
#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <filesystem>
#include <mutex>

#include <fontconfig/fontconfig.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "imgui.h"

inline std::mutex g_application_lock;

// TODO: add some error handling in here?
/// @brief calls a function whenever a file is modified.
inline void watch_file(const std::filesystem::path& path, std::invocable auto fn) {

    // vim will only reliably produce IN_MOVE_SELF events, so we have to catch those
    auto flags = IN_MODIFY | IN_CLOSE_WRITE | IN_MOVE_SELF;

    int fd = inotify_init();
    assert(fd != -1);

    int wd = inotify_add_watch(fd, path.c_str(), flags);
    assert(wd != -1);

    while (true) {
        struct inotify_event event;
        ssize_t bytes_read = read(fd, &event, sizeof event);
        assert(bytes_read != -1);
        assert(bytes_read != 0);

        // vim will actually swap the edited file with a new file, so
        // we have to catch that and add the file back to the watchlist
        if (event.mask & IN_IGNORED) {
            while (true) {
                wd = inotify_add_watch(fd, path.c_str(), flags);
                if (wd != -1) break;
                assert(errno == ENOENT);
            }
        }

        fn();
    }

    assert(inotify_rm_watch(fd, wd) != -1);
    assert(close(fd) != -1);
}

[[nodiscard]] inline auto parse_font_name(const char* font_name) -> std::optional<std::filesystem::path> {

    FcInit();
    FcConfig* conf = FcInitLoadConfigAndFonts();
    FcPattern* pattern = FcNameParse(reinterpret_cast<const FcChar8*>(font_name));
    if (pattern == nullptr) return {};

    FcConfigSubstitute(conf, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;
   	FcPattern* font = FcFontMatch(conf, pattern, &result);
    if (font == nullptr) return {};

    FcChar8* file = nullptr;
    if (FcPatternGetString(font, FC_FILE, 0, &file) != FcResultMatch)
        return {};

    std::filesystem::path path = reinterpret_cast<const char*>(file);

    FcPatternDestroy(font);
    FcPatternDestroy(pattern);
    FcConfigDestroy(conf);
    FcFini();
    return path;
}

template <typename T>
class string_switch {
    public:
    constexpr explicit string_switch(std::string_view string)
    : m_string(string)
    { }

    constexpr string_switch& match(std::string_view query, T value) {
        if (m_string == query)
            m_value = std::move(value);

        return *this;
    }

    constexpr string_switch& catchall(T value) {
        if (!m_value)
            m_value = std::move(value);

        return *this;
    }

    constexpr string_switch& if_empty(std::invocable auto fn) {
        if (!m_value)
            fn();

        return *this;
    }

    // throws std::bad_optional_access if there is no matched value.
    [[nodiscard]] constexpr T done() const {
        return *m_value;
    }

    [[nodiscard]] constexpr std::optional<T> maybe_done() const {
        return m_value;
    }

    private:
    const std::string_view m_string;
    std::optional<T> m_value;

};

consteval void test_string_switch() {

    constexpr int a = string_switch<int>("bar")
        .match("foo", 1)
        .match("bar", 2)
        .match("baz", 3)
        .done();
    static_assert(a == 2);

    constexpr int b = string_switch<int>("qux")
        .match("foo", 1)
        .match("bar", 2)
        .match("baz", 3)
        .catchall(45)
        .done();
    static_assert(b == 45);

}

[[nodiscard]] constexpr inline auto parse_color_string(std::string_view string) -> std::optional<ImVec4> {

    auto color = string_switch<ImVec4>(string)
        .match("transparent", ImVec4(0, 0, 0, 0))
        .match("black", ImVec4(0, 0, 0, 1))
        .match("white", ImVec4(1, 1, 1, 1))
        .match("red",   ImVec4(1, 0, 0, 1))
        .match("green", ImVec4(0, 1, 0, 1))
        .match("blue",  ImVec4(0, 0, 1, 1))
        .maybe_done();

    if (color) return *color;

    if (string.length() != 1+8 && string.length() != 1+6) return {};
    if (string.at(0) != '#') return {};

    uint32_t value = 0;
    auto err = std::from_chars(string.data()+1, string.data()+string.size(), value, 16).ec;
    if (err != std::errc{}) return {};

    if (string.length() == 1+8)
        return ImVec4(
            (value >> 3*8 & 0xff) / 255.0f,
            (value >> 2*8 & 0xff) / 255.0f,
            (value >> 1*8 & 0xff) / 255.0f,
            (value >> 0*8 & 0xff) / 255.0f
        );
    else if (string.length() == 1+6)
        return ImVec4(
            (value >> 2*8 & 0xff) / 255.0f,
            (value >> 1*8 & 0xff) / 255.0f,
            (value >> 0*8 & 0xff) / 255.0f,
            1.0f
        );
    else
        assert(!"string length should have been checked by now");

}

consteval void test_parse_color_string() {

    constexpr auto a = parse_color_string("#00000000");
    static_assert(a.has_value());
    static_assert(a->x == 0);
    static_assert(a->y == 0);
    static_assert(a->z == 0);
    static_assert(a->w == 0);

    constexpr auto b = parse_color_string("#ffffffff");
    static_assert(b.has_value());
    static_assert(b->x == 1);
    static_assert(b->y == 1);
    static_assert(b->z == 1);
    static_assert(b->w == 1);

    constexpr auto c = parse_color_string("#ffffff");
    static_assert(c.has_value());
    static_assert(c->x == 1);
    static_assert(c->y == 1);
    static_assert(c->z == 1);
    static_assert(c->w == 1);

    constexpr auto d = parse_color_string("transparent");
    static_assert(d.has_value());
    static_assert(d->x == 0);
    static_assert(d->y == 0);
    static_assert(d->z == 0);
    static_assert(d->w == 0);

    constexpr auto e = parse_color_string("red");
    static_assert(e.has_value());
    static_assert(e->x == 1);
    static_assert(e->y == 0);
    static_assert(e->z == 0);
    static_assert(e->w == 1);

    constexpr auto f = parse_color_string("white");
    static_assert(f.has_value());
    static_assert(f->x == 1);
    static_assert(f->y == 1);
    static_assert(f->z == 1);
    static_assert(f->w == 1);

}
