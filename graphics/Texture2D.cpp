#include "Texture2D.h"

#include "spdlog/spdlog.h"
#include <spdlog/fmt/bundled/color.h>

Texture2D::Texture2D()
{
  glCreateTextures(GL_TEXTURE_2D, 1, &handle);
}

Texture2D::~Texture2D()
{
  glDeleteTextures(1, &handle);
}

void Texture2D::Use() const
{
  glBindTexture(GL_TEXTURE_2D, GetHandle());
}

void Texture2D::SetActiveUnit(uint32_t active_texture_unit)
{
  activeTexture = active_texture_unit;
  glActiveTexture(GL_TEXTURE0 + active_texture_unit);
  glBindTexture(GL_TEXTURE_2D, GetHandle());
}

bool Texture2D::Load(const uint8_t *image_data, int32_t image_width, int32_t image_height, GLint internal_image_format, GLenum data_format)
{
  return Load(image_data, image_width, image_height, internal_image_format, data_format, 0);
}

bool Texture2D::Load(const uint8_t *image_data, int32_t image_width, int32_t image_height, GLint internal_image_format, GLenum data_format, uint32_t active_texture_unit)
{
  bool success = true;

  if (image_data)
  {
    width = image_width;
    height = image_height;
    internalFormat = internal_image_format;
    dataFormat = data_format;

    SetActiveUnit(active_texture_unit);
    Use();

    glTextureParameteri(GetHandle(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(GetHandle(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(GetHandle(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(GetHandle(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTextureStorage2D(GetHandle(), 1, internalFormat, width, height);

    Update(image_data, width, height);
  }
  else
  {
    success = false;
  }

  return success;
}

void Texture2D::Update(const uint8_t *image_data, const int32_t image_width, const int32_t image_height) const
{
  glTextureSubImage2D(handle, 0, 0, 0, image_width, image_height, GL_RGB, GL_UNSIGNED_BYTE, image_data);
}

bool Texture2D::Copy(const Texture2D &copy_texture) const
{
  return Copy(copy_texture, 0, 0, 0, 0, 0, 0);
}

bool Texture2D::Copy(const Texture2D &copy_texture, const GLint src_level, const GLint src_x, const GLint src_y, const GLint dst_level, const GLint dst_x, const GLint dst_y) const
{
  glCopyImageSubData(handle, GL_TEXTURE_2D, src_level, src_x, src_y, 0, copy_texture.GetHandle(), GL_TEXTURE_2D, dst_level, dst_x, dst_y, 0, width, height, GL_FALSE);

  const GLint error = glGetError();
  if (error != GL_NO_ERROR)
  {
    spdlog::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Unable to copy texture {}", copy_texture.GetHandle()));
  }

  return (error == GL_NO_ERROR);
}

uint32_t Texture2D::GetActiveTextureUnit() const
{
  return activeTexture;
}

uint32_t Texture2D::GetHandle() const
{
  return handle;
}

int32_t Texture2D::GetWidth() const
{
  return width;
}

int32_t Texture2D::GetHeight() const
{
  return height;
}

int32_t Texture2D::GetBytesPerPixel() const
{
  int32_t bytes_per_pixel = 1;

  if (dataFormat == GL_RGB)
  {
    bytes_per_pixel = 3;
  }
  else if (dataFormat == GL_RGBA)
  {
    bytes_per_pixel = 4;
  }

  return bytes_per_pixel;
}
