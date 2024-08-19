#pragma once

#include <string>
#include <fstream>

#include <glad/glad.h>

enum class ShaderType {VERTEX, FRAGMENT};
enum class ShaderTypeError {none, invalid};

class OGLShader
{
  public:
    OGLShader() = delete;
    [[maybe_unused]] explicit OGLShader(std::ifstream& file);
    [[maybe_unused]] explicit OGLShader(std::ifstream& file, ShaderType shader_type);
    [[maybe_unused]] explicit OGLShader(const std::string& shader_code, ShaderType shader_type);
    ~OGLShader();

    void Use() const;

  private:
    GLenum handle = 0u;
};
