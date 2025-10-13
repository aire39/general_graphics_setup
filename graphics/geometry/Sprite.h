#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Primitive.h"
#include "../images/Texture2D.h"

class Texture2D;
struct SDL_Surface;

class Sprite
{
  public:
    Sprite();
    explicit Sprite(const std::string& new_name);
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

    virtual int32_t GetWidth() const;
    virtual int32_t GetHeight() const;
    virtual int32_t GetBytesPerPixel() const;

    virtual void Draw();

    void SetName(const std::string& new_name);
    [[nodiscard]] std::string GetName() const;
    [[nodiscard]] uint32_t GetID() const;
    static uint32_t TotalNumber();

  protected:
    std::vector<primitive::Vertex> vertices;
    std::vector<GLuint> indices;
    std::string name;
    uint32_t id = 0;
    glm::fvec3 position {0.0f, 0.0f, 0.0f};
    float rotation {0.0f};
    glm::fvec2 scale {1.0f, 1.0f};
    GLuint vaoHandle = 0;
    GLuint vboHandle = 0;
    GLuint iboHandle = 0;

    Texture2D texture;

    static inline uint32_t refCount = 0;
    static inline uint32_t refCountId = 0;

  private:
    void Init();
    void Update();
};
