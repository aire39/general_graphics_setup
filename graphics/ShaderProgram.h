#pragma once

#include <map>
#include <vector>
#include <span>
#include "OGLShader.h"

#include <glad/glad.h>

class ShaderProgram
{
  public:
    ShaderProgram();
    ~ShaderProgram();

    [[nodiscard]] GLuint GetProgramId() const;

    void AttachShader(const OGLShader & shader);
    void AttachShader(std::span<const OGLShader> shader_list);
    void AttachShader(std::vector<std::reference_wrapper<const OGLShader>> && shader_list);
    void Use() const;

    void SetBool(const std::string &name, bool value) const;
    void SetInt(const std::string &name, int value) const;
    void SetFloat(const std::string &name, float value) const;

private:
    void DetachShaders(const OGLShader & shader) const;
    void DetachShaders(std::span<const OGLShader> & shader_list) const;
    void DetachShaders(std::vector<std::reference_wrapper<const OGLShader>> & shader_list) const;
    void LinkShaders();

    GLuint handle = 0;
    GLchar programStatusLog[512] {'\0'};
    std::map<OGLShader::ShaderType, const OGLShader*> shaderList;
};
