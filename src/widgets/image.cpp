#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "image.hpp"

namespace widgets {

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
        ImGuiHelpers::Center(size.x);
        ImGui::Image(m_texture_id, size);
    }

} // namespace widgets