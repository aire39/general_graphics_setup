#include "Sprite.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>

namespace {
    constexpr size_t num_sprite_verts = 4;
    constexpr size_t num_sprite_indices = 4;
}

Sprite::Sprite()
 : vertices(num_sprite_verts)
 , indices(num_sprite_indices)
{
  Init();
}

[[maybe_unused]] void Sprite::SetPosition(glm::vec2 pos)
{
  position = {pos.x, pos.y, position.z};
}

[[maybe_unused]] void Sprite::SetPosition(glm::vec3 pos)
{
  position = pos;
}

void Sprite::Draw()
{
  glBindVertexArray(vaoHandle);
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);

  glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
}

void Sprite::Init()
{
  const uint32_t layout_data_size = sizeof(primitive::Vertex) * vertices.size();
  spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "Sprite size: {} bytes", layout_data_size));
  vertices[0] = {.position = {-0.5f, -0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}, .uvcoords_0 = {0.0f, 0.0f}, .uvcoords_1 = {0.0f, 0.0f}};
  vertices[1] = {.position = { 0.5f, -0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}, .uvcoords_0 = {1.0f, 0.0f}, .uvcoords_1 = {1.0f, 0.0f}};
  vertices[2] = {.position = { 0.5f,  0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}, .uvcoords_0 = {1.0f, 1.0f}, .uvcoords_1 = {1.0f, 1.0f}};
  vertices[3] = {.position = {-0.5f,  0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}, .uvcoords_0 = {0.0f, 1.0f}, .uvcoords_1 = {0.0f, 1.0f}};

  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 3;
  indices[3] = 2;

  glGenVertexArrays(1, &vaoHandle);
  glBindVertexArray(vaoHandle);
  spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "vao handle id: {}", vaoHandle));

  glGenBuffers(1, &vboHandle);
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
  spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "vbo handle id: {}", vboHandle));
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(primitive::Vertex) * vertices.size()), vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(primitive::Vertex), nullptr);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(primitive::Vertex), (GLvoid*)sizeof(primitive::Vertex::position));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(primitive::Vertex), (GLvoid*)(sizeof(primitive::Vertex::position) + sizeof(primitive::Vertex::color)));
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(primitive::Vertex), (GLvoid*)(sizeof(primitive::Vertex::position) + sizeof(primitive::Vertex::color) + sizeof(primitive::Vertex::uvcoords_0)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

  glGenBuffers(1, &iboHandle);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
  spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "ibo handle id: {}", iboHandle));
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_sprite_indices, indices.data(), GL_STATIC_DRAW);
}
