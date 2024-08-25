#include "OGLShader.h"

#include <magic_enum.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>

[[maybe_unused]] OGLShader::OGLShader([[maybe_unused]]const std::string& file_path)
{
  std::ifstream file (file_path);
  if (file)
  {
    std::string file_extension = file_path.substr(file_path.find_last_of('.'));

    if(file_extension.contains("vert") || file_extension.contains("frag"))
    {
      std::string shader_code;

      std::string text_line;
      while (std::getline(file, text_line))
      {
        shader_code += text_line + "\n";
      }

      if (!shader_code.empty())
      {
        if (file_extension.contains("vert"))
        {
          shaderType = ShaderType::VERTEX;
        }
        else if (file_extension.contains("frag"))
        {
          shaderType = ShaderType::FRAGMENT;
        }

        Init(shader_code);
        if (Compile())
        {
          isShaderCompiled = true;
        }
      }
      else
      {
        spdlog::error(fmt::format(fmt::fg(fmt::terminal_color::bright_red), "No valid shader code! Shader code is empty!"));
      }
    }
    else
    {
      spdlog::error(fmt::format(fmt::fg(fmt::terminal_color::bright_red), "No valid shader file extension ({}) was found!", file_extension));
    }

    file.close();
  }
  else
  {
    spdlog::error(fmt::format(fmt::fg(fmt::terminal_color::bright_red), "No valid file ({}) for shader opened!", file_path));
  }
}

[[maybe_unused]] OGLShader::OGLShader([[maybe_unused]]const std::string& shader_code, [[maybe_unused]]ShaderType shader_type)
  : shaderType (shader_type)
{
  if (!shader_code.empty())
  {
    Init(shader_code);
    if (Compile())
    {
      isShaderCompiled = true;
    }
  }
  else
  {
    spdlog::error(fmt::format(fmt::fg(fmt::terminal_color::bright_red), "No valid shader ({}) code! Shader code is empty!", magic_enum::enum_name(shader_type)));
  }
}

OGLShader::~OGLShader()
{
  glDeleteShader(handle);
  handle = 0;
  isShaderCompiled = false;
}

[[maybe_unused]] GLuint OGLShader::GetShaderId() const
{
  return handle;
}

[[maybe_unused]] GLuint OGLShader::HasShaderCompiled() const
{
  return isShaderCompiled;
}

[[maybe_unused]] OGLShader::ShaderType OGLShader::GetShaderType() const
{
  return shaderType;
}


void OGLShader::Init(const std::string& shader_code)
{
  switch (shaderType)
  {
    case ShaderType::VERTEX:
      handle = glCreateShader(GL_VERTEX_SHADER);
      break;

    case ShaderType::FRAGMENT:
      handle = glCreateShader(GL_FRAGMENT_SHADER);
      break;
  }

  const auto shader_type_name = magic_enum::enum_name(shaderType);
  spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "Created Shader {} ({})", shader_type_name.data(), handle));

  const char * sc_str = shader_code.c_str();
  glShaderSource(handle, 1, &sc_str, nullptr);
}

bool OGLShader::Compile()
{
  glCompileShader(handle);

  GLint compile_status = 0;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &compile_status);

  if(compile_status == GL_FALSE)
  {
    GLint max_length = 0;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &max_length);
    glGetShaderInfoLog(handle, max_length, &max_length, compileStatusLog);

    spdlog::error(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "shader compile error:\n{}", compileStatusLog));
  }
  else
  {
    spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_green), "compiled successfully!"));
  }

  return (compile_status != 0);
}
