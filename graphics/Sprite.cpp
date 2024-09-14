#include "Sprite.h"

Sprite::Sprite()
 : vertices(4)
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

void Sprite::Init()
{
  vertices[0] = {.position = {0.0f, 0.0f, 0.0f}};
  vertices[1] = {.position = {1.0f, 0.0f, 0.0f}};
  vertices[2] = {.position = {1.0f, 1.0f, 0.0f}};
  vertices[3] = {.position = {0.0f, 1.0f, 0.0f}};
}
