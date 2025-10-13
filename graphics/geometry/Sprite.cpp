#include "Sprite.h"

#include "common/support/logging.h"
#include <spdlog/fmt/bundled/color.h>
#include "SDL3_image/SDL_image.h"

namespace {
    constexpr size_t num_sprite_vertices = 4;
    constexpr size_t num_sprite_indices = 4;
    constexpr bool allow_logging = false;
}

Sprite::Sprite()
 : vertices(num_sprite_vertices)
 , indices(num_sprite_indices)
{
  name = "Sprite";
  Sprite::id++;
  id = refCountId++;

  glGenVertexArrays(1, &vaoHandle);
  glGenBuffers(1, &vboHandle);
  glGenBuffers(1, &iboHandle);

  Init();
  Update();
}

Sprite::Sprite(const std::string& new_name)
  : Sprite()
{
  SetName(new_name);
}

Sprite::~Sprite()
{
  Sprite::id--;
  glDeleteBuffers(1, &iboHandle);
  glDeleteBuffers(1, &vboHandle);
  glDeleteVertexArrays(1, &vaoHandle);
}

[[maybe_unused]] void Sprite::SetPosition(glm::vec2 pos)
{
  position = {pos.x, pos.y, position.z};
}

[[maybe_unused]] void Sprite::SetPosition(const glm::vec3 pos)
{
  position = pos;
}

void Sprite::SetRotation(float rot)
{
  rotation = rot;
}

void Sprite::SetScale(glm::vec2 s)
{
  scale = s;
}

glm::fvec3 Sprite::GetPosition() const
{
  return position;
}

void Sprite::SetColor(const glm::vec3 color)
{
  for (auto & v : vertices)
  {
    v.color = color;
  }

  Update();
}

void Sprite::LoadTexture(const std::string& image_file)
{
  const SDL_Surface* image = IMG_Load(image_file.c_str());
  LoadTexture(image);
}

void Sprite::LoadTexture(const SDL_Surface* image)
{
  if (image)
  {
    texture.Load(static_cast<uint8_t *>(image->pixels), image->w, image->h, GL_RGB8, GL_RGB);
  }
  else
  {
    logging::error("No valid image loaded!");
  }
}

bool Sprite::CopyTexture(const Texture2D &copy_texture) const
{
  return texture.Copy(copy_texture);
}

const Texture2D *Sprite::GetTexture() const
{
  return &texture;
}

int32_t Sprite::GetWidth() const
{
  return texture.GetWidth();
}

int32_t Sprite::GetHeight() const
{
  return texture.GetHeight();
}

int32_t Sprite::GetBytesPerPixel() const
{
  return texture.GetBytesPerPixel();
}

void Sprite::Draw()
{
  glBindVertexArray(vaoHandle);
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);

  glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Sprite::SetName(const std::string &new_name)
{
  name = new_name;
}

std::string Sprite::GetName() const
{
  return name;
}

uint32_t Sprite::GetID() const
{
  return id;
}

uint32_t Sprite::TotalNumber()
{
  return refCount;
}

void Sprite::Init()
{
  [[maybe_unused]] const size_t layout_data_size = sizeof(primitive::Vertex) * vertices.size();
  // ReSharper disable once CppDFAUnreachableCode NOLINTNEXTLINE(clang-diagnostic-unreachable-code)
  if constexpr (allow_logging) logging::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "Sprite size: {} bytes", layout_data_size));
  vertices[0] = {.position = {-0.5f, -0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}, .uvcoords_0 = {0.0f, 0.0f}, .uvcoords_1 = {0.0f, 0.0f}};
  vertices[1] = {.position = { 0.5f, -0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}, .uvcoords_0 = {1.0f, 0.0f}, .uvcoords_1 = {1.0f, 0.0f}};
  vertices[2] = {.position = { 0.5f,  0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}, .uvcoords_0 = {1.0f, 1.0f}, .uvcoords_1 = {1.0f, 1.0f}};
  vertices[3] = {.position = {-0.5f,  0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}, .uvcoords_0 = {0.0f, 1.0f}, .uvcoords_1 = {0.0f, 1.0f}};

  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 3;
  indices[3] = 2;
}

void Sprite::Update()
{
  glBindVertexArray(vaoHandle);
  // ReSharper disable once CppDFAUnreachableCode NOLINTNEXTLINE(clang-diagnostic-unreachable-code)
  if constexpr (allow_logging) logging::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "VAO handle id: {}", vaoHandle));

  glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
  // ReSharper disable once CppDFAUnreachableCode NOLINTNEXTLINE(clang-diagnostic-unreachable-code)
  if constexpr (allow_logging) logging::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "VBO handle id: {}", vboHandle));
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(primitive::Vertex) * vertices.size()), vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(primitive::Vertex), nullptr);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(primitive::Vertex), reinterpret_cast<GLvoid *>(sizeof(primitive::Vertex::position)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(primitive::Vertex), reinterpret_cast<GLvoid *>(sizeof(primitive::Vertex::position) + sizeof(primitive::Vertex::color)));
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(primitive::Vertex), reinterpret_cast<GLvoid *>(sizeof(primitive::Vertex::position) + sizeof(primitive::Vertex::color) + sizeof(primitive::Vertex::uvcoords_0)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
  // ReSharper disable once CppDFAUnreachableCode NOLINTNEXTLINE(clang-diagnostic-unreachable-code)
  if constexpr (allow_logging) logging::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "IBO handle id: {}", iboHandle));
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_sprite_indices, indices.data(), GL_STATIC_DRAW);
}
