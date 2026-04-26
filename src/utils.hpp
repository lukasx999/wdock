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

[[nodiscard]] constexpr inline auto parse_color_string(std::string_view string) -> std::optional<ImVec4> {

    if (string.length() != 9) return {};
    if (string.at(0) != '#') return {};

    uint32_t value = 0;
    auto err = std::from_chars(string.data()+1, string.data()+string.size(), value, 16).ec;

    if (err != std::errc{}) return {};

    return ImVec4(
        (value >> 3*8 & 0xff) / 255.0f,
        (value >> 2*8 & 0xff) / 255.0f,
        (value >> 1*8 & 0xff) / 255.0f,
        (value >> 0*8 & 0xff) / 255.0f
    );
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

}

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
    constexpr explicit string_switch(std::string string)
    : m_string(std::move(string))
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

    private:
    const std::string m_string;
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