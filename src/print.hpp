#pragma once

#include <format>
#include <iostream>
#include <print>

inline constexpr auto g_color_red         = "\033[0;31m";
inline constexpr auto g_color_blue        = "\033[0;34m";
inline constexpr auto g_color_green       = "\033[0;32m";
inline constexpr auto g_color_bold_red    = "\033[1;31m";
inline constexpr auto g_color_bold_blue   = "\033[1;34m";
inline constexpr auto g_color_bold_green  = "\033[1;32m";
inline constexpr auto g_color_bold_yellow = "\033[1;33m";
inline constexpr auto g_color_end         = "\033[0m";

#ifndef NDEBUG
#define DBG(value) std::println(std::cerr, "{}: {}", #value, value)
#endif // NDEBUG

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

template <typename... Args>
inline void print_warning(std::format_string<Args...> fmt, Args&&... args) {
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    std::println(std::cerr, "{}WARNING{}: {}", g_color_bold_yellow, g_color_end, msg);
}