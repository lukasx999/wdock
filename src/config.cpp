#include <fstream>
#include <ranges>

#include "config.hpp"

namespace {

// TODO: handle type errors
void parse_window(const kdl::Node& node, struct config::window& window) {

    for (auto& child : node.children()) {
        auto name = string_from_u8(child.name());
        auto& args = child.args();

        if (name == "font") {
            window.font = string_from_u8(args[0].as<std::u8string>());

        } else if (name == "fontsize") {
            window.fontsize = args[0].as<float>();

        } else if (name == "background-color") {
            window.background_color = string_from_u8(args[0].as<std::u8string>());

        } else if (name == "border-radius") {
            window.border_radius = args[0].as<float>();

        } else if (name == "padding") {
            window.padding = args[0].as<float>();

        } else if (name == "size") {
            auto& props = child.properties();

            if (!props.contains(u8"width") || !props.contains(u8"height"))
                throw config_error("property \"size\" must specify both \"width\" and \"height\".");

            int width = props.at(u8"width").as<int>();
            int height = props.at(u8"height").as<int>();
            window.size = { width, height };

        } else if (name == "anchor") {
            auto anchor_str = args[0].as<std::u8string>();

            if (anchor_str == u8"top")
                window.anchor = window::anchor::top;

            else if (anchor_str == u8"right")
                window.anchor = window::anchor::right;

            else if (anchor_str == u8"bottom")
                window.anchor = window::anchor::bottom;

            else if (anchor_str == u8"left")
                window.anchor = window::anchor::left;

            else throw config_error("invalid anchor value");

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
        def.properties[string_from_u8(child.name())] = child.args();
    }

}

void parse_widgets(const kdl::Node& node, config& config) {
    config.widgets = node.args()
        | std::views::transform(&kdl::Value::as<std::u8string>)
        | std::ranges::to<std::vector<std::string>>();
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
            parse_window(node, config.window);

        else if (name == "widgets")
            parse_widgets(node, config);

        else if (name == "define-widget")
            parse_widget_definition(node, config);

        else
            throw config_error(std::format("invalid config option: \"{}\"", name));

    }

    return config;
}