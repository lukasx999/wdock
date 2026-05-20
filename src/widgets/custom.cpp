#include "custom.hpp"

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

        if (!m_label.empty()) {
            ImGui::TextUnformatted(m_label.c_str());
            ImGui::SameLine();
        }
        ImGui::TextUnformatted(buf.c_str());

        assert(pclose(file) != -1);
    }

} // namespace widgets