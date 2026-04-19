#include <fstream>
#include <ranges>

#include "config.hpp"
#include "utils.hpp"
#include "widgets.hpp"

// TODO: handle type errors

namespace {

[[nodiscard]] struct config::window parse_window(const kdl::Node& node) {
    struct config::window window;

    for (auto& child : node.children()) {
        auto name = string_from_u8(child.name());
        auto& args = child.args();

        if (name == "font") {
            window.style.font = string_from_u8(args[0].as<std::u8string>());

        } else if (name == "fontsize") {
            window.style.fontsize = args[0].as<float>();

        } else if (name == "background-color") {
            window.style.background_color = string_from_u8(args[0].as<std::u8string>());

        } else if (name == "border-radius") {
            window.style.border_radius = args[0].as<float>();

        } else if (name == "padding") {
            window.style.padding = args[0].as<float>();

        } else if (name == "size") {
            auto& props = child.properties();

            if (!props.contains(u8"width") || !props.contains(u8"height"))
                throw config_error("property \"size\" must specify both \"width\" and \"height\".");

            int width = props.at(u8"width").as<int>();
            int height = props.at(u8"height").as<int>();
            window.size = { width, height };

        } else if (name == "anchor") {

            std::string anchor;
            try {
                anchor = string_from_u8(args[0].as<std::u8string>());
            } catch (const kdl::TypeError&) {
                throw config_error("\"anchor\" value must be of type string.");
            }

            window.anchor = string_switch<window::anchor>(anchor)
                .match("top",    window::anchor::top)
                .match("right",  window::anchor::right)
                .match("bottom", window::anchor::bottom)
                .match("left",   window::anchor::left)
                .if_empty([&] {
                    throw config_error(std::format("invalid \"anchor\" value: \"{}\" (must be one of \"top\", \"right\", \"bottom\" or \"left\" )", anchor));
                })
                .done();

        } else if (name == "layer") {
            auto layer = args[0].as<std::u8string>();

            if (layer == u8"background")
                window.layer = window::layer::background;

            else if (layer == u8"bottom")
                window.layer = window::layer::bottom;

            else if (layer == u8"top")
                window.layer = window::layer::top;

            else if (layer == u8"overlay")
                window.layer = window::layer::overlay;

            else
                throw config_error(std::format("invalid \"layer\" value: \"{}\"", string_from_u8(layer)));

        } else if (name == "margin") {
            auto& props = child.properties();

            if (!props.contains(u8"top") ||  !props.contains(u8"right") ||  !props.contains(u8"bottom") ||  !props.contains(u8"left"))
                throw config_error("property \"margin\" must specify \"top\", \"right\", \"bottom\" and \"left\".");

            int top    = props.at(u8"top").as<int>();
            int right  = props.at(u8"right").as<int>();
            int bottom = props.at(u8"bottom").as<int>();
            int left   = props.at(u8"left").as<int>();
            window.margin = { top, right, bottom, left };

        } else
            throw config_error(std::format("invalid window option \"{}\"", name));

    }

    return window;
}

void parse_widget_definition(const kdl::Node& node, config& config) {
    std::string name = string_from_u8(node.args()[0].as<std::u8string>());

    if (config.widget_definitions.contains(name))
        throw config_error(std::format("widget named \"{}\" has been defined multiple times", name));

    auto& def = config.widget_definitions[name];

    try {
        def.preset = string_from_u8(node.properties().at(u8"preset").as<std::u8string>());
    } catch (const std::out_of_range&) {
        throw config_error("widget definition must include \"preset\" property");
    }

    for (auto& child : node.children()) {
        def.props[string_from_u8(child.name())] = child.args();
    }

}

[[nodiscard]] auto parse_widgets(const kdl::Node& node) -> std::vector<std::string> {
    return node.args()
        | std::views::transform(&kdl::Value::as<std::u8string>)
        | std::ranges::to<std::vector<std::string>>();
}

[[nodiscard]] auto parse_datetime(const config::widget_definition::properties& props) -> std::unique_ptr<widgets::datetime> {

    std::string timezone = "Europe/Vienna";
    std::string format = "%d.%m.%Y";

    for (auto& [name, values] : props) {

        if (name == "timezone")
            timezone = string_from_u8(values.front().as<std::u8string>());

        else if (name == "format")
            format = string_from_u8(values.front().as<std::u8string>());

        else
            throw config_error(std::format("property \"{}\" does not exist in widget \"datetime\".", name));
    }

    return std::make_unique<widgets::datetime>(timezone, format);
}

void parse_widgets(config& config) {

    for (auto& widget_name : config.widgets) {

        if (!config.widget_definitions.contains(widget_name))
            throw config_error(std::format("widget \"{}\" has not been defined.", widget_name));

        auto widget_def = config.widget_definitions.at(widget_name);
        auto preset = widget_def.preset;
        auto& props = widget_def.props;

        if (preset == "datetime") {
            config.used_widgets.push_back(parse_datetime(props));

        } else if (preset == "image") {
            auto path = string_from_u8(props["path"].front().as<std::u8string>());
            auto scaling = props["scaling"].front().as<float>();

            config.used_widgets.push_back(std::make_unique<widgets::image>(path, scaling));

        } else if (preset == "kernel") {
            config.used_widgets.push_back(std::make_unique<widgets::kernel>());

        } else if (preset == "button") {
            auto label = string_from_u8(props["label"].front().as<std::u8string>());
            auto on_click = string_from_u8(props["on_click"].front().as<std::u8string>());

            config.used_widgets.push_back(std::make_unique<widgets::button>(label, on_click));

        } else
            throw std::runtime_error(std::format("widget preset \"{}\" does not exist.", widget_def.preset));

    }

}

} // namespace

config parse_config(const std::filesystem::path& config_path) {

    std::ifstream stream(config_path);
    std::u8string config_src(std::istreambuf_iterator<char>(stream), {});
    auto doc = kdl::parse(config_src);

    config config;

    for (auto& node : doc.nodes()) {
        auto name = string_from_u8(node.name());

        if (name == "window")
            config.window =  parse_window(node);

        else if (name == "widgets")
            config.widgets =  parse_widgets(node);

        else if (name == "define-widget")
            parse_widget_definition(node, config);

        else
            throw config_error(std::format("invalid config option: \"{}\"", name));

    }

    parse_widgets(config);

    return config;
}