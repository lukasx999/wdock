#pragma once

#include <utility>
#include <cassert>
#include <optional>
#include <string_view>
#include <filesystem>

#include "imgui.h"
#include "imgui_stdlib.h"

#include "print.hpp"

namespace ImGuiHelpers {

    inline void SameLineRightAligned(float width) {
        float offset = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width;
        ImGui::SameLine();
        ImGui::SetCursorPosX(offset);
    }

    inline void Center(float width, float alignment=0.5f) {
        float total_width = ImGui::GetContentRegionAvail().x;
        float offset = (total_width - width) * alignment;
        if (offset >= 0.0f)
            ImGui::SetCursorPosX(offset);
    }

} // namespace ImGuiHelpers

inline void replace_substring(std::string& string, std::string_view query, std::string_view replacement) {
    size_t offset = 0;
    while (true) {
        size_t pos = string.find(query, offset);
        if (pos == std::string::npos)
            break;
        string.replace(pos, query.length(), replacement);
    }
}

/// @return whether the operation was successful
bool download_file(const char* url, const std::filesystem::path& path);

[[nodiscard]] auto parse_font_name(const char* font_name) -> std::optional<std::filesystem::path>;

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