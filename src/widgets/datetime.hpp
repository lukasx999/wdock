#pragma once

#include <string>

#include "../widget.hpp"

namespace widgets {

    class datetime : public widget {
        public:
        datetime(widget_style style, std::string timezone, std::string format);

        void on_draw() const override;

        private:
        const std::string m_timezone;
        const std::string m_format;

        [[nodiscard]] std::string get_formatted_time() const;

    };

} // namespace widgets