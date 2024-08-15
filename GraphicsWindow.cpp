#include "GraphicsWindow.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL3/SDL_opengl.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>

[[maybe_unused]] GraphicsWindow::GraphicsWindow(const std::string& window_title, int32_t window_width, int32_t window_height)
  : GraphicsWindow(window_title, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)
{
}

GraphicsWindow::GraphicsWindow(const std::string& window_title, int32_t window_width, int32_t window_height, SDL_WindowFlags window_flags)
  : sdlWindow (
    std::unique_ptr<SDL_Window, void (*)(SDL_Window*)>(
      SDL_CreateWindow(window_title.c_str(),
                       window_width,
                       window_height,
                       window_flags),
                       [](SDL_Window *sdl_window) -> void { GraphicsWindow::DestroyWindow(sdl_window); }
    )
  )
  ,windowWidth (window_width)
  ,windowHeight (window_height)
{
  if (((window_flags & SDL_WINDOW_OPENGL) != 0) && (glContext == nullptr))
  {
    spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_blue), "Create OpenGLContext!"));
    glContext = std::make_unique<OpenGLContext>(*this);
    SetOpenGLContext(glContext);
  }
}

GraphicsWindow::~GraphicsWindow()
{
  if (glContext)
  {
    glContext.reset();
  }

  spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_magenta), "Destroy GraphicsWindow!"));
}

void GraphicsWindow::SetOpenGLContext(std::unique_ptr<OpenGLContext> && gl_context) noexcept
{
  glContext = std::move(gl_context);

  SetBackgoundColor(backgroundColor);
  glViewport(0, 0, windowWidth, windowHeight);
}

void GraphicsWindow::SetOpenGLContext(std::unique_ptr<OpenGLContext> & gl_context)
{
  glContext = std::move(gl_context);

  SetBackgoundColor(backgroundColor);
  glViewport(0, 0, windowWidth, windowHeight);
}

void GraphicsWindow::SetBackgoundColor(math_types::float4 background_color)
{
  glClearColor(background_color.x, background_color.y, background_color.z, background_color.w);
}

void GraphicsWindow::ClearWindow()
{
  glClear(GL_COLOR_BUFFER_BIT);
}

bool GraphicsWindow::IsWindowInitialized() const
{
  return (sdlWindow != nullptr);
}

SDL_Window * GraphicsWindow::GetSDLWindow() const
{
  return sdlWindow.get();
}

OpenGLContext * GraphicsWindow::GetOpenGLContext() const
{
  return glContext.get();
}

void GraphicsWindow::SwapBuffers() const
{
  SDL_GL_SwapWindow(sdlWindow.get());
}

void GraphicsWindow::Destroy()
{
  sdlWindow.reset();
}

void GraphicsWindow::DestroyWindow(SDL_Window* sdl_window)
{
  SDL_DestroyWindow(sdl_window);
}
