#include "ShaderProgram.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>

#include <magic_enum.hpp>

ShaderProgram::ShaderProgram()
{
  handle = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{

  glDeleteProgram(handle);
}

GLuint ShaderProgram::GetProgramId() const
{
  return handle;
}

void ShaderProgram::AttachShader(const OGLShader & shader)
{
  DetachShaders(shader);
  shaderList.insert({shader.GetShaderType(), &shader});

  // check to see if at least vertex and fragment shaders exist before attempting to link

  bool has_basic_vertex_shader = (shaderList.find(OGLShader::ShaderType::VERTEX) != shaderList.end());
  bool has_basic_fragment_shader = (shaderList.find(OGLShader::ShaderType::FRAGMENT) != shaderList.end());

  if (has_basic_vertex_shader && has_basic_fragment_shader)
  {
    LinkShaders();
  }
  else
  {
    spdlog::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "program shader ({}): may be missing vertex and/or fragment shaders!", handle));
  }
}

void ShaderProgram::AttachShader(std::span<const OGLShader> shader_list)
{
  DetachShaders(shader_list);

  for (const auto & shader : shader_list )
  {
    shaderList.insert({shader.GetShaderType(), &shader});
  }

  // check to see if at least vertex and fragment shaders exist before attempting to link

  bool has_basic_vertex_shader = (shaderList.find(OGLShader::ShaderType::VERTEX) != shaderList.end());
  bool has_basic_fragment_shader = (shaderList.find(OGLShader::ShaderType::FRAGMENT) != shaderList.end());

  if (has_basic_vertex_shader && has_basic_fragment_shader)
  {
    LinkShaders();
  }
  else
  {
    spdlog::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "program shader ({}): may be missing vertex and/or fragment shaders!", handle));
  }
}

void ShaderProgram::AttachShader(std::vector<std::reference_wrapper<const OGLShader>> && shader_list)
{
  DetachShaders(shader_list);

  for (const auto & shader : shader_list )
  {
    shaderList.insert({shader.get().GetShaderType(), &shader.get()});
  }

  // check to see if at least vertex and fragment shaders exist before attempting to link

  bool has_basic_vertex_shader = (shaderList.find(OGLShader::ShaderType::VERTEX) != shaderList.end());
  bool has_basic_fragment_shader = (shaderList.find(OGLShader::ShaderType::FRAGMENT) != shaderList.end());

  if (has_basic_vertex_shader && has_basic_fragment_shader)
  {
    LinkShaders();
  }
  else
  {
    spdlog::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "program shader ({}): may be missing vertex and/or fragment shaders!", handle));
  }
}

void ShaderProgram::Use() const
{
  glUseProgram(handle);
}

void ShaderProgram::SetBool(const std::string &name, bool value) const
{
  glUniform1i(glGetUniformLocation(handle, name.c_str()), (int)value);
}
void ShaderProgram::SetInt(const std::string &name, int value) const
{
  glUniform1i(glGetUniformLocation(handle, name.c_str()), value);
}
void ShaderProgram::SetFloat(const std::string &name, float value) const
{
  glUniform1f(glGetUniformLocation(handle, name.c_str()), value);
}

void ShaderProgram::DetachShaders(const OGLShader & shader) const
{
  glDetachShader(handle, shader.GetShaderId());
}

void ShaderProgram::DetachShaders(std::span<const OGLShader> & shader_list) const
{
  for(const auto& shader : shader_list)
  {
    glDetachShader(handle, shader.GetShaderId());
  }
}

void ShaderProgram::DetachShaders(std::vector<std::reference_wrapper<const OGLShader>> & shader_list) const
{
  for(const auto& shader : shader_list)
  {
    glDetachShader(handle, shader.get().GetShaderId());
  }
}

void ShaderProgram::LinkShaders()
{
  for(const auto [s_type, shader] : shaderList)
  {
    glAttachShader(handle, shader->GetShaderId());
  }

  glLinkProgram(handle);

  GLint link_status = GL_FALSE;
  glGetProgramiv(handle, GL_LINK_STATUS, &link_status);

  if (!link_status)
  {
    GLint max_length = 0;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &max_length);

    glGetProgramInfoLog(handle, max_length, &max_length, programStatusLog);

    spdlog::error(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "shader compile error:\n{}", programStatusLog));
  }
  else
  {
    std::string shader_type_list_str;
    for (const auto & [s_type, shader] : shaderList)
    {
      shader_type_list_str += std::string(magic_enum::enum_name(s_type)) + ",";
    }
    shader_type_list_str[shader_type_list_str.size()-1] = '\0';

    spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_green), "program shader ({}): [{}] linked successfully!", handle, shader_type_list_str));
  }
}