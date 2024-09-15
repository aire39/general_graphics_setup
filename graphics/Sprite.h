#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Primitive.h"

class Sprite
{
  public:
    Sprite();
    ~Sprite() = default;

   [[maybe_unused]] void SetPosition(glm::vec2 pos);
   [[maybe_unused]] void SetPosition(glm::vec3 pos);

    virtual void Draw();

  protected:
    std::vector<primitive::Vertex> vertices;
    std::vector<GLuint> indices;
    glm::fvec3 position {0.0f, 0.0f, 0.0f};
    GLuint vaoHandle = 0;
    GLuint vboHandle = 0;
    GLuint iboHandle = 0;

  private:
    void Init();
};
