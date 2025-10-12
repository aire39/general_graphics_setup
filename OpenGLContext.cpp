#include "OpenGLContext.h"

#include "GraphicsWindow.h"

#include "common/support/logging.h"
#include <spdlog/fmt/bundled/color.h>

#include "SDL3/SDL_opengl.h"

namespace {
  constexpr int32_t default_ogl_version_major {4};
  constexpr int32_t default_ogl_version_minor {5};
}

OpenGLContext::OpenGLContext(GraphicsWindow & graphics_window)
  : OpenGLContext(graphics_window, default_ogl_version_major, default_ogl_version_minor)
{
}

OpenGLContext::OpenGLContext(GraphicsWindow & graphics_window, const int32_t ogl_version_major, const int32_t ogl_version_minor)
{
  SDL_GL_LoadLibrary(nullptr);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, ogl_version_major);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, ogl_version_minor);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

  context = SDL_GL_CreateContext(graphics_window.GetSDLWindow());
}

OpenGLContext::~OpenGLContext()
{
  logging::info(fmt::format(fmt::fg(fmt::terminal_color::bright_magenta), "Destroy OpenGLContext!"));
  SDL_GL_DestroyContext(context);
}

SDL_GLContext OpenGLContext::GetContext() const
{
  return context;
}

void OpenGLContext::Flush()
{
  glFlush();
}

void OpenGLContext::Finish()
{
  glFinish();
}
