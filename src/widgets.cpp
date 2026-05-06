#include "widgets.hpp"

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace widgets {

    void custom::on_draw() const {
        FILE* file = popen(m_command.c_str(), "r");
        if (file == nullptr)
            throw widget_error("error executing command: \"{}\"", m_command);

        std::string buf;

        char c;
        while ((c = fgetc(file)) != EOF)
            buf += c;

        // TODO: check for exit code
        if (not feof(file))
            throw widget_error("error reading output from command: \"{}\"", m_command);

        ImGui::TextUnformatted(buf.c_str());

        assert(pclose(file) != -1);
    }

    image::image(widget_style style, const std::filesystem::path& path, float scaling)
    : widget(style)
    , m_scaling(scaling)
    {
        int channels;
        unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 0);
        if (data == nullptr)
            throw widget_error("failed to load image at {}", path.c_str());

        GLenum format = [&] {
            switch (channels) {
                case 3: return GL_RGB;
                case 4: return GL_RGBA;
                default:
                stbi_image_free(data);
                throw widget_error("invalid amount of channels ({})", channels);
            }
        }();

        glGenTextures(1, &m_texture_id);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    image::~image() {
        // TODO: cant delete texture after drawing with imgui because of deferred drawing in draw list
        // glDeleteTextures(1, &m_texture_id);
    }

    void image::on_draw() const {
        ImVec2 size(m_width * m_scaling, m_height * m_scaling);
        ImGui::Image(m_texture_id, size);
    }

    void memory::on_draw() const {
        std::ifstream file("/proc/meminfo");
        std::string line;

        int64_t total = -1;
        int64_t avail = -1;

        while (std::getline(file, line)) {
            auto [attribute, value] = parse_proc_meminfo_line(line);

            if (attribute == "MemTotal")
                total = value;

            else if (attribute == "MemAvailable")
                avail = value;

        }

        assert(total != -1);
        assert(avail != -1);

        int64_t used = total - avail;

        auto gibs = 1 / std::pow(2, 20);
        auto fmt = std::format("{:.1f}Gib/{:.1f}Gib", used * gibs, total * gibs);
        float frac = static_cast<float>(used) / total;

        ImGui::TextUnformatted(fmt.c_str());
        ImGui::SameLine();
        ImGui::ProgressBar(frac, {0, 0}, m_show_percentage ? nullptr : "");

    }

    auto memory::parse_proc_meminfo_line(std::string_view line) -> std::tuple<std::string, uint64_t> {

        size_t colon_pos = line.find(':');
        assert(colon_pos != std::string::npos);
        std::string attribute(line.substr(0, colon_pos));

        size_t value_start = line.find_first_not_of(' ', colon_pos+1);
        assert(value_start != std::string::npos);

        size_t value_end = line.find_first_of(' ', value_start);

        // some entries dont have a "kB" at the end, in that case the value
        // ends at the end of the line.
        size_t n = value_end == std::string::npos
            ? std::string::npos
            : value_end - value_start;

        auto value_string = line.substr(value_start, n);
        uint64_t value;

        auto ec = std::from_chars(value_string.data(), value_string.data() + value_string.size(), value).ec;
        assert(ec == std::errc{});

        return std::make_tuple(attribute, value);
    }

    void disk::on_draw() const {
        struct statvfs buf;
        assert(statvfs("/", &buf) == 0);

        auto gibs = 1 / std::pow(2, 30);
        auto size = buf.f_frsize;

        uint64_t total = size * buf.f_blocks;
        uint64_t free = size * buf.f_bfree;
        uint64_t used = total - free;

        auto fmt = std::format(" {:.1f}GiB/{:.1f}GiB", used * gibs, total * gibs);

        ImGui::TextUnformatted(fmt.c_str());
        ImGui::SameLine();
        ImGui::ProgressBar(static_cast<float>(used) / total, {0, 0}, m_show_percentage ? nullptr : "");
    }

    std::string datetime::get_formatted_time() const {
        try {
            auto now = std::chrono::system_clock::now();
            std::chrono::zoned_time zt(m_timezone, now);

            time_t time = std::chrono::system_clock::to_time_t(zt);
            tm* tm = localtime(&time);

            std::stringstream fmt;
            fmt << std::put_time(tm, m_format.c_str());
            return fmt.str();

        } catch (const std::runtime_error&) {
            throw widget_error("invalid time zone: {}", m_timezone);
        }

    }

} // namespace widgets