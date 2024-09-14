#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Primitive.h"

class Sprite
{
  public:
    Sprite();
    ~Sprite() = default;

   [[maybe_unused]] void SetPosition(glm::vec2 pos);
   [[maybe_unused]] void SetPosition(glm::vec3 pos);

    virtual void Draw() = 0;

  protected:
    std::vector<primitive::Vertex> vertices;
    glm::fvec3 position {0.0f, 0.0f, 0.0f};

  private:
    void Init();
};
