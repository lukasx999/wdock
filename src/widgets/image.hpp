#pragma once

#include <glad/gl.h>

#include "../widget.hpp"

namespace widgets {

    class image : public widget {
        public:
        image(widget_style style, const std::filesystem::path& path, float scaling);

        ~image();
        image(const image&) = delete;
        image(image&&) noexcept = delete;
        image& operator=(const image&) = delete;
        image& operator=(image&&) noexcept = delete;

        void on_draw() const override;

        private:
        const float m_scaling;
        int m_width;
        int m_height;
        GLuint m_texture_id = 0;

    };

} // namespace widgets