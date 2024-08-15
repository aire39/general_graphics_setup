#pragma once

#include <SDL3/SDL.h>

class GraphicsWindow;

class OpenGLContext
{
  public:
    OpenGLContext() = delete;
    explicit OpenGLContext(GraphicsWindow & graphics_window);
    ~OpenGLContext();

    [[nodiscard]] SDL_GLContext GetContext() const;

  private:
    SDL_GLContext context;
};
