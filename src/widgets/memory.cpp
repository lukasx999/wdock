#include <fstream>
#include <cmath>

#include "memory.hpp"

namespace widgets {

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
        auto fmt = std::format("mem: {:.1f}Gib/{:.1f}Gib", used * gibs, total * gibs);
        float frac = static_cast<float>(used) / total;

        ImGui::TextUnformatted(fmt.c_str());
        ImGui::SameLine();
        ImGui::ProgressBar(frac, {300, 32}, m_show_percentage ? nullptr : "");

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

        [[maybe_unused]] auto ec = std::from_chars(value_string.data(), value_string.data() + value_string.size(), value).ec;
        assert(ec == std::errc{});

        return std::make_tuple(attribute, value);
    }

} // namespace widgets