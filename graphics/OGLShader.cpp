#include "OGLShader.h"

#include <expected>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>

namespace {
  std::expected<GLenum, ShaderTypeError> GetShaderType(ShaderType shader_type)
  {
    std::expected<GLenum, ShaderTypeError> expected;

    switch (shader_type)
    {
      case ShaderType::VERTEX:
        expected = std::expected<GLenum, ShaderTypeError>(GL_VERTEX_SHADER);
        break;

      default:
        expected = std::unexpected<ShaderTypeError>(ShaderTypeError::invalid);
        break;
    }

    return expected;
  }
}

[[maybe_unused]] OGLShader::OGLShader(std::ifstream& file)
{
  // based on the extension of the file it can automatically determine the type of shader
  // vert --> vertex shader
  // frag/pixel --> fragment shader
  // geom --> geometry shader
  // tess --> tesselation shader

  if (file)
  {

  }
}

[[maybe_unused]] [[maybe_unused]] OGLShader::OGLShader([[maybe_unused]]std::ifstream& file, [[maybe_unused]]ShaderType shader_type)
{

  auto shader = GetShaderType(shader_type);

  if (shader.has_value())
  {
    handle = glCreateShader(shader.value());
  }
}

[[maybe_unused]] OGLShader::OGLShader([[maybe_unused]]const std::string& shader_code, [[maybe_unused]]ShaderType shader_type)
{
  auto shader = GetShaderType(shader_type);

  if (shader.has_value())
  {
    handle = glCreateShader(shader.value());
    spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "Created Shader X ({})", handle));
  }
}

OGLShader::~OGLShader()
{
  handle = 0;
}

void OGLShader::Use() const
{
  glUseProgram(handle);
}
