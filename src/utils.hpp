#pragma once

#include <utility>
#include <optional>
#include <string>
#include <string_view>

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