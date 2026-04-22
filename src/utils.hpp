#pragma once

#include <utility>
#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <filesystem>

#include <sys/stat.h>
#include <fontconfig/fontconfig.h>

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

[[nodiscard]] inline bool has_file_changed(const std::filesystem::path& path) {
    static auto prev_last_access = 0;

    struct stat buf;
    assert(stat(path.c_str(), &buf) == 0);
    auto last_access = buf.st_atim.tv_nsec;

    bool has_changed = last_access != prev_last_access;

    prev_last_access = last_access;

    return has_changed;
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