#pragma once

#include <utility>
#include <functional>
#include <cassert>
#include <iostream>
#include <optional>
#include <string_view>
#include <filesystem>
#include <mutex>

#include "imgui.h"

// this lock exists, so that we can make sure that the main thread is not
// rendering to the window, while the config watcher thread tries to reload the config.
inline std::mutex g_draw_lock;

inline constexpr auto g_color_red        = "\033[0;31m";
inline constexpr auto g_color_blue       = "\033[0;34m";
inline constexpr auto g_color_green      = "\033[0;32m";
inline constexpr auto g_color_bold_red   = "\033[1;31m";
inline constexpr auto g_color_bold_blue  = "\033[1;34m";
inline constexpr auto g_color_bold_green = "\033[1;32m";
inline constexpr auto g_color_end        = "\033[0m";

#ifndef NDEBUG
#define DBG(value) std::println(std::cerr, "{}: {}", #value, value)
#endif // NDEBUG

inline void imgui_center(float width, float alignment=0.5f) {
    float total_width = ImGui::GetContentRegionAvail().x;
    float offset = (total_width - width) * alignment;
    ImGui::SetCursorPosX(offset);
}

/// @return whether the operation was successful
bool download_file(const char* url, const std::filesystem::path& path);

/// @brief calls a function whenever a file is modified.
/// @return returns false if the watcher could not be installed
bool watch_file(const std::filesystem::path& path, std::function<void()> fn);

[[nodiscard]] auto parse_font_name(const char* font_name) -> std::optional<std::filesystem::path>;

template <typename... Args>
inline void print_info(std::format_string<Args...> fmt, Args&&... args) {
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    std::println(std::cerr, "{}INFO{}: {}", g_color_bold_blue, g_color_end, msg);
}

template <typename... Args>
inline void print_debug([[maybe_unused]] std::format_string<Args...> fmt, [[maybe_unused]] Args&&... args) {
    #ifndef NDEBUG
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    std::println(std::cerr, "{}DEBUG{}: {}", g_color_bold_green, g_color_end, msg);
    #endif // NDEBUG
}

template <typename... Args>
inline void print_error(std::format_string<Args...> fmt, Args&&... args) {
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    std::println(std::cerr, "{}ERROR{}: {}", g_color_bold_red, g_color_end, msg);
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

    /// @throws std::bad_optional_access if there is no matched value.
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

    static_assert(string_switch<int>("bar")
        .match("foo", 1)
        .match("bar", 2)
        .match("baz", 3)
        .done() == 2);

    static_assert(string_switch<int>("qux")
        .match("foo", 1)
        .match("bar", 2)
        .match("baz", 3)
        .catchall(45)
        .done() == 45);

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

    else {
        assert(!"string length should have been checked by now");
        std::unreachable();
    }

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