#ifdef _WIN32
#include <windows.h>
#endif

#include <string>

#include <glad/glad.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "GraphicsWindow.h"
#include "graphics/OGLShader.h"

int32_t WindowResize(void * data, SDL_Event * event);

int32_t main([[maybe_unused]]int32_t argc, [[maybe_unused]]char*argv[])
{
  #ifdef _WIN32
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle != INVALID_HANDLE_VALUE) {
      DWORD mode = 0;
      if (GetConsoleMode(handle, &mode)) {
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(handle, mode);
      }
    }
  #endif

  spdlog::info("hello graphics world!");

  // sdl and window initialization

  SDL_Init(SDL_INIT_VIDEO);

  constexpr static std::string_view window_name = "Graphics Window";
  constexpr int32_t window_width  = 800;
  constexpr int32_t window_height = 600;
  constexpr uint64_t sdl_window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

  auto graphics_window = GraphicsWindow(window_name.data(), window_width, window_height, sdl_window_flags);
  SDL_GL_MakeCurrent(graphics_window.GetSDLWindow(), graphics_window.GetOpenGLContext()->GetContext());

  if (!graphics_window.IsWindowInitialized())
  {
    spdlog::error("Unable to initialize a window!!!");
    return 0;
  }

  gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress));

  // this is a quick test for calling opengl functions
  const std::string vertex_shader_code = "#version 330 core\n" \
                                         "layout (location = 0) in vec3 aPos; // the position variable has attribute position 0\n" \
                                         "  \n" \
                                         "out vec4 vertexColor; // specify a color output to the fragment shader\n" \
                                         "\n" \
                                         "void main()\n" \
                                         "{\n" \
                                         "    gl_Position = vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor\n" \
                                         "    vertexColor = vec4(0.5, 0.0, 0.0, 1.0); // set the output variable to a dark-red color\n" \
                                         "}";

  OGLShader vshader(vertex_shader_code, OGLShader::ShaderType::VERTEX);

  const std::string fragment_shader_code = "#version 330 core\n" \
                                           "out vec4 FragColor;\n" \
                                           "  \n" \
                                           "in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  \n" \
                                           "\n" \
                                           "void main()\n" \
                                           "{\n" \
                                           "    FragColor = vertexColor;\n" \
                                           "}";

  OGLShader fshader(fragment_shader_code, OGLShader::ShaderType::FRAGMENT);

  // initialize imGUI
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  const char * glsl_version = "#version 150";
  ImGui_ImplSDL3_InitForOpenGL(graphics_window.GetSDLWindow(), graphics_window.GetOpenGLContext()->GetContext());
  ImGui_ImplOpenGL3_Init(glsl_version);

  SDL_AddEventWatch(WindowResize, graphics_window.GetSDLWindow());

  // event handling

  bool is_running = true;
  SDL_Event sdl_event;
  while (is_running)
  {
    while (SDL_PollEvent(&sdl_event))
    {
      ImGui_ImplSDL3_ProcessEvent(&sdl_event);

      if (sdl_event.type == SDL_EVENT_QUIT)
      {
        is_running = false;
      }
    }

    SDL_Delay(16);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // do imgui work

    ImGui::Render();
    GraphicsWindow::ClearWindow();

    // do work

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    graphics_window.SwapBuffers();
  }

  // cleanup

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  graphics_window.Destroy();
  SDL_Quit();

  return 0;
}

int32_t WindowResize(void * data, SDL_Event * event)
{
  if (event->window.type == SDL_EVENT_WINDOW_RESIZED)
  {
    SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);
    if (window == static_cast<SDL_Window*>(data))
    {
      spdlog::info("window resizing...");
    }
  }

  return 0;
}
