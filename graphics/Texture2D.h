#pragma once

#include <cstdint>
#include <glad/glad.h>

class Texture2D
{
  public:
    Texture2D();
    ~Texture2D();

    void Use() const;
    void SetActiveUnit(uint32_t active_texture_unit);
    bool Load(const uint8_t* image_data, int32_t image_width, int32_t image_height, GLint internal_image_format, GLenum data_format);
    bool Load(const uint8_t* image_data, int32_t image_width, int32_t image_height, GLint internal_image_format, GLenum data_format, uint32_t active_texture_unit);
    void Update(const uint8_t* image_data, int32_t image_width, int32_t image_height) const;
    [[nodiscard]] bool Copy(const Texture2D& copy_texture) const;
    [[nodiscard]] bool Copy(const Texture2D& copy_texture, GLint src_level, GLint src_x, GLint src_y, GLint dst_level, GLint dst_x, GLint dst_y) const;
    [[nodiscard]] uint32_t GetActiveTextureUnit() const;
    [[nodiscard]] uint32_t GetHandle() const;
    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;
    [[nodiscard]] int32_t GetBytesPerPixel() const;

  private:
    uint32_t handle = 0;
    int32_t width = 0;
    int32_t height = 0;
    uint32_t activeTexture = 0;
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat = GL_RGB;
};
