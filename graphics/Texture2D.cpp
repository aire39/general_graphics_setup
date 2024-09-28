#include "Texture2D.h"

Texture2D::Texture2D()
{
  glGenTextures(1, &handle);
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
}

bool Texture2D::Load(const uint8_t *image_data, int32_t image_width, int32_t image_height, GLint image_format)
{
  return Load(image_data, image_width, image_height, image_format, 0);
}

bool Texture2D::Load(const uint8_t *image_data, int32_t image_width, int32_t image_height, GLint image_format, uint32_t active_texture_unit)
{
  bool success = true;

  if (image_data)
  {
    width = image_width;
    height = image_height;

    SetActiveUnit(active_texture_unit);
    Use();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, image_format, width, height, GL_FALSE, image_format, GL_UNSIGNED_BYTE, image_data);
  }
  else
  {
    success = false;
  }

  return success;
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
