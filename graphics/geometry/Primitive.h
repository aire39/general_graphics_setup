#pragma once

#include <glm/glm.hpp>

namespace primitive {
  struct Vertex
  {
    glm::vec3 position {0.0f, 0.0f, 0.0f};
    glm::vec3 color {0.0f, 0.0f, 0.0f};
    glm::vec2 uvcoords_0 {0.0f, 0.0f};
    glm::vec2 uvcoords_1 {0.0f, 0.0f};
  };
}
