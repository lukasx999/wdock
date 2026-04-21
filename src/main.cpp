#include <cstdlib>
#include <iostream>
#include <format>
#include <print>

#include "application.hpp"
#include "widgets.hpp"
#include "window.hpp"
#include "config.hpp"

#include <sys/stat.h>

[[nodiscard]] bool has_file_changed(const std::filesystem::path& path) {
    static auto prev_last_access = 0;

    struct stat buf;
    assert(stat(path.c_str(), &buf) == 0);
    auto last_access = buf.st_atim.tv_nsec;

    bool has_changed = last_access != prev_last_access;

    prev_last_access = last_access;

    return has_changed;
}

int main() {

    auto config_path = "config.kdl";

    try {
        application app;

        // parse_config() must be called AFTER the application has been constructed, because it
        // may call widget constructors, which may call into OpenGL functions.
        config config = parse_config(config_path);
        app.load_config(config);
        app.run();

    } catch (const config_error& error) {
        std::println(std::cerr, "failed to parse config file: {}", error.what());
        return EXIT_FAILURE;

    } catch (const window_error& error) {
        std::println(std::cerr, "failed to open window: {}", error.what());
        return EXIT_FAILURE;

    } catch (const widget_error& error) {
        std::println(std::cerr, "failed to add widget: {}", error.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}