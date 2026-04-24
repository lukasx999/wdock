#include <cstdlib>
#include <iostream>
#include <format>
#include <print>
#include <thread>

#include "application.hpp"
#include "widgets.hpp"
#include "window.hpp"
#include "config.hpp"

int main() {
    window a("a", 500, 500);
    window b("b", 500, 500);

    a.set_anchor(window::anchor::left);
    b.set_anchor(window::anchor::right);

    a.on_draw([] {
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    });

    b.on_draw([] {
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    });

    std::jthread ta([&] {
        a.run();
    });

    std::jthread tb([&] {
        b.run();
    });

    // std::vector<const window*> windows = { &a, &b };
    // window::run_concurrent(windows);

    return 0;
}

int main2() {

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