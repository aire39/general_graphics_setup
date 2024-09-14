#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include <SDL3/SDL.h>

#include "math/MTypes.h"
#include "OpenGLContext.h"

class GraphicsWindow
{
  public:
    GraphicsWindow() = delete;

    [[maybe_unused]] GraphicsWindow(const std::string& window_title, int32_t window_width, int32_t window_height);
    [[maybe_unused]] GraphicsWindow(const std::string& window_title, int32_t window_width, int32_t window_height, SDL_WindowFlags window_flags);
    ~GraphicsWindow();

    void SetOpenGLContext(std::unique_ptr<OpenGLContext> && gl_context) noexcept;
    void SetOpenGLContext(std::unique_ptr<OpenGLContext> & gl_context);
    static void SetBackgroundColor(math_types::float4 background_color);
    static void ClearWindow();

    [[nodiscard]] bool IsWindowInitialized() const;

    [[nodiscard]] SDL_Window * GetSDLWindow() const;
    [[nodiscard]] OpenGLContext * GetOpenGLContext() const;

    void SwapBuffers() const;

    void Destroy();

  private:
    static inline bool windowInitialized = false;

    static void DestroyWindow(SDL_Window*);
    std::unique_ptr<SDL_Window, void (*)(SDL_Window*)> sdlWindow;

    int32_t windowWidth = -1;
    int32_t windowHeight = -1;
    math_types::float4 backgroundColor {0.4f, 0.4f, 0.9f, 1.0f};
    [[maybe_unused]]std::unique_ptr<OpenGLContext> glContext;
};
