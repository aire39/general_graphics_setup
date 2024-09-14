#pragma once

#include <cstdint>
#include <SDL3/SDL.h>

class GraphicsWindow;

class OpenGLContext
{
  public:
    OpenGLContext() = delete;
    explicit OpenGLContext(GraphicsWindow & graphics_window);
    explicit inline OpenGLContext(GraphicsWindow & graphics_window, int32_t ogl_version_major, int32_t ogl_version_minor);
    ~OpenGLContext();

    [[nodiscard]] SDL_GLContext GetContext() const;

  private:
    SDL_GLContext context = nullptr;
};
