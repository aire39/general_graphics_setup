#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Primitive.h"
#include "Texture2D.h"

class Texture2D;
class SDL_Surface;

class Sprite
{
  public:
    Sprite();
    virtual ~Sprite();

    [[maybe_unused]] void SetPosition(glm::vec2 pos);
    [[maybe_unused]] void SetPosition(glm::vec3 pos);
    [[maybe_unused]] void SetRotation(float rot);
    [[maybe_unused]] void SetScale(glm::vec2 s);
    [[nodiscard]] [[maybe_unused]] glm::fvec3 GetPosition() const;

    [[maybe_unused]] void SetColor(glm::vec3 color);

    virtual void LoadTexture(const std::string& image_file);
    virtual void LoadTexture(const SDL_Surface* image);
    [[nodiscard]] bool CopyTexture(const Texture2D& copy_texture) const;

    [[nodiscard]] const Texture2D *GetTexture() const;

    virtual void Draw();

  protected:
    std::vector<primitive::Vertex> vertices;
    std::vector<GLuint> indices;
    glm::fvec3 position {0.0f, 0.0f, 0.0f};
    float rotation {0.0f};
    glm::fvec2 scale {1.0f, 1.0f};
    GLuint vaoHandle = 0;
    GLuint vboHandle = 0;
    GLuint iboHandle = 0;

    Texture2D texture;

  private:
    void Init();
    void Update();
};
