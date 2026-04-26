#include <fstream>
#include <ranges>

#include <kdl/kdl.h>
#include <kdlpp.h>

#include "config.hpp"
#include "utils.hpp"
#include "widgets.hpp"

// TODO: handle type errors

namespace {

    struct widget_definition {
        using properties = std::unordered_map<std::string, std::vector<kdl::Value>>;
        std::string preset;
        properties props;
        widget_style style;
    };

    [[nodiscard]] std::string string_from_u8(const std::u8string& str) {
        return { str.begin(), str.end() };
    }

    [[nodiscard]] auto parse_widget_datetime(const widget_definition& def) -> std::unique_ptr<widgets::datetime> {

        std::string timezone = "Europe/Vienna";
        std::string format = "%d.%m.%Y";

        for (auto& [name, values] : def.props) {

            if (name == "timezone")
                timezone = string_from_u8(values.front().as<std::u8string>());

            else if (name == "format")
                format = string_from_u8(values.front().as<std::u8string>());

            else
                throw config_error("property \"{}\" does not exist in widget \"datetime\".", name);
        }

        return std::make_unique<widgets::datetime>(def.style, std::move(timezone), std::move(format));
    }

    [[nodiscard]] auto parse_widget_image(const widget_definition& def) -> std::unique_ptr<widgets::image> {

        std::optional<std::string> path;
        float scaling = 1.0f;

        for (auto& [name, values] : def.props) {

            if (name == "path")
                path = string_from_u8(values.front().as<std::u8string>());

            else if (name == "scaling")
                scaling = values.front().as<float>();

            else
                throw config_error("property \"{}\" does not exist in widget \"image\".", name);

        }

        if (!path)
            throw config_error("property \"path\" in widget preset \"image\" does not have a default value.");

        return std::make_unique<widgets::image>(def.style, *path, scaling);
    }

    [[nodiscard]] auto parse_widget_button(const widget_definition& def) -> std::unique_ptr<widgets::button> {

        std::optional<std::string> label;
        std::optional<std::string> on_click;

        for (auto& [name, values] : def.props) {
            if (name == "label")
                label = string_from_u8(values.front().as<std::u8string>());

            else if (name == "on_click")
                on_click = string_from_u8(values.front().as<std::u8string>());

            else
                throw config_error("property \"{}\" does not exist in widget \"button\".", name);

        }

        if (!label)
            throw config_error("property \"label\" in widget preset \"button\" does not have a default value.");

        if (!on_click)
            throw config_error("property \"on_click\" in widget preset \"button\" does not have a default value.");

        return std::make_unique<widgets::button>(def.style, *label, *on_click);
    }
    [[nodiscard]] auto parse_widget_custom(const widget_definition& def) -> std::unique_ptr<widgets::custom> {

        std::optional<std::string> command;

        for (auto& [name, values] : def.props) {
            if (name == "command")
                command = string_from_u8(values.front().as<std::u8string>());
            else
                throw config_error("property \"{}\" does not exist in widget \"custom\".", name);
        }

        if (!command)
            throw config_error("property \"command\" in widget preset \"custom\" does not have a default value.");

        return std::make_unique<widgets::custom>(def.style, *command);
    }

    [[nodiscard]] auto parse_widget_memory(const widget_definition& def) -> std::unique_ptr<widgets::memory> {
        static_cast<void>(def);

        // for (auto& [name, values] : props) {
        // if (name == "...")
        // else
        //     throw config_error("property \"{}\" does not exist in widget \"memory\".", name);
        // }

        return std::make_unique<widgets::memory>(def.style);
    }

    [[nodiscard]] auto parse_widgets(std::span<const widget_definition> widget_definitions) -> std::vector<std::unique_ptr<widget>> {
        std::vector<std::unique_ptr<widget>> widgets;

        for (auto& def : widget_definitions) {
            auto preset = def.preset;

            // TODO: string_switch
            if (preset == "datetime")
                widgets.push_back(parse_widget_datetime(def));

            else if (preset == "image")
                widgets.push_back(parse_widget_image(def));

            else if (preset == "system")
                widgets.push_back(std::make_unique<widgets::system_info>(def.style));

            else if (preset == "button")
                widgets.push_back(parse_widget_button(def));

            else if (preset == "custom")
                widgets.push_back(parse_widget_custom(def));

            else if (preset == "memory")
                widgets.push_back(parse_widget_memory(def));

            else if (preset == "disk")
                widgets.push_back(std::make_unique<widgets::disk>(def.style));

            else
                throw config_error("widget preset \"{}\" does not exist.", preset);

        }

        return widgets;
    }

    [[nodiscard]] struct config::window::style parse_style_window(const kdl::Node& node) {

        struct config::window::style style;

        for (auto& child : node.children()) {
            auto args = child.args();
            auto name = string_from_u8(child.name());

            if (name == "font")
                style.font = string_from_u8(args.front().as<std::u8string>());

            else if (name == "fontsize")
                style.fontsize = args.front().as<float>();

            else if (name == "background-color")
                style.background_color = string_from_u8(args.front().as<std::u8string>());

            else if (name == "border-radius")
                style.border_radius = args.front().as<float>();

            else if (name == "padding")
                style.padding = args.front().as<float>();

            else
                throw config_error("invalid window style property \"{}\".", name);

        }

        return style;
    }

    [[nodiscard]] struct config::window parse_window(const kdl::Node& node) {
        struct config::window window;

        for (auto& child : node.children()) {
            auto name = string_from_u8(child.name());
            auto& args = child.args();

            if (name == "size") {
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
                        throw config_error("invalid \"anchor\" value: \"{}\" (must be one of \"top\", \"right\", \"bottom\" or \"left\" )", anchor);
                    })
                    .done();

            } else if (name == "layer") {
                auto layer = string_from_u8(args[0].as<std::u8string>());

                window.layer = string_switch<window::layer>(layer)
                    .match("background", window::layer::background)
                    .match("bottom",     window::layer::bottom)
                    .match("top",        window::layer::top)
                    .match("overlay",    window::layer::overlay)
                    .if_empty([&] {
                        throw config_error("invalid \"layer\" value: \"{}\"", layer);
                    })
                    .done();

            } else if (name == "margin") {
                auto& props = child.properties();

                if (!props.contains(u8"top") ||  !props.contains(u8"right") ||  !props.contains(u8"bottom") ||  !props.contains(u8"left"))
                    throw config_error("property \"margin\" must specify \"top\", \"right\", \"bottom\" and \"left\".");

                int top    = props.at(u8"top").as<int>();
                int right  = props.at(u8"right").as<int>();
                int bottom = props.at(u8"bottom").as<int>();
                int left   = props.at(u8"left").as<int>();
                window.margin = { top, right, bottom, left };

            } else if (name == "style") {
                window.style = parse_style_window(child);

            } else
                throw config_error("invalid window option \"{}\"", name);

        }

        return window;
    }

    [[nodiscard]] widget_style parse_style_widget(const kdl::Node& node) {
        widget_style style;

        for (auto& child : node.children()) {
            auto args = child.args();
            auto name = string_from_u8(child.name());

            if (name == "frame_padding")
                style.frame_padding = args.front().as<float>();

            else if (name == "frame_rounding")
                style.frame_rounding = args.front().as<float>();

            else
                throw config_error("unknown style property \"{}\"", name);
        }

        return style;
    }

    [[nodiscard]] auto parse_widget_definition(const kdl::Node& node) -> std::pair<std::string, widget_definition> {
        std::string name = string_from_u8(node.args().front().as<std::u8string>());

        widget_definition def;

        try {
            def.preset = string_from_u8(node.properties().at(u8"preset").as<std::u8string>());
        } catch (const std::out_of_range&) {
            throw config_error("widget definition must include \"preset\" property");
        }

        if (node.properties().size() != 1)
            throw config_error("widget definition must only include a single property called \"preset\"");

        for (auto& child : node.children()) {
            if (child.name() == u8"style")
                def.style = parse_style_widget(child);
            else
                def.props[string_from_u8(child.name())] = child.args();
        }

        return { name, def };
    }

    [[nodiscard]] auto parse_widget_declaration(const kdl::Node& node) -> std::vector<std::string> {
        return node.args()
        | std::views::transform(&kdl::Value::as<std::u8string>)
        | std::ranges::to<std::vector<std::string>>();
    }

} // namespace

// TODO: put all of this stuff into a config_parser class?
config parse_config(const std::filesystem::path& config_path) {

    if (not std::filesystem::exists(config_path))
        throw config_error("config file at \"{}\" does not exist.", config_path.c_str());

    std::ifstream stream(config_path);
    std::u8string config_src(std::istreambuf_iterator<char>(stream), {});
    auto doc = kdl::parse(config_src);

    config config;

    std::vector<std::string> used_widgets;
    std::unordered_map<std::string, widget_definition> widget_definitions;

    for (auto& node : doc.nodes()) {
        auto name = string_from_u8(node.name());

        if (name == "window")
            // TODO: check for multiple definitions
            config.window = parse_window(node);

        else if (name == "declare-widgets") {
            if (not used_widgets.empty())
                throw config_error("there may only be one \"declare-widgets\" definition.");

            used_widgets = parse_widget_declaration(node);

        } else if (name == "define-widget") {
            auto [name, def] = parse_widget_definition(node);

            if (widget_definitions.contains(name))
                throw config_error("widget \"{}\" has been defined multiple times.", name);

            widget_definitions.insert({name, def});

        } else
            throw config_error("invalid config option: \"{}\"", name);

    }

    auto widgets = used_widgets
        | std::views::transform([&](const std::string& name) {
            if (not widget_definitions.contains(name))
                throw config_error("widget \"{}\" has not been defined.", name);
            return widget_definitions[name];
        })
        | std::ranges::to<std::vector<widget_definition>>();

    config.widgets = parse_widgets(widgets);

    return config;
}