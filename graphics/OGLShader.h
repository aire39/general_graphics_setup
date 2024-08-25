#pragma once

#include <cstdint>
#include <string>
#include <fstream>

#include <glad/glad.h>

class OGLShader
{
  public:
    enum class ShaderType : int8_t {VERTEX, FRAGMENT};

    OGLShader() = delete;
    [[maybe_unused]] explicit OGLShader(const std::string& file_path);
    [[maybe_unused]] explicit OGLShader(const std::string& shader_code, ShaderType shader_type);
    ~OGLShader();

    [[maybe_unused]] [[nodiscard]] GLuint GetShaderId() const;
    [[maybe_unused]] [[nodiscard]] GLuint HasShaderCompiled() const;
    [[maybe_unused]] [[nodiscard]] ShaderType GetShaderType() const;

  private:
    GLuint handle = 0;
    GLchar compileStatusLog[512] {'\0'};
    bool isShaderCompiled = false;
    ShaderType shaderType;

    void Init(const std::string& shader_code);
    bool Compile();
};
