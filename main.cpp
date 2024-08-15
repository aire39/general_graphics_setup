#include <iostream>
#include <string>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <SDL3/SDL.h>

#include "GraphicsWindow.h"

int32_t WindowResize(void * data, SDL_Event * event);

int32_t main([[maybe_unused]]int32_t argc, [[maybe_unused]]char*argv[])
{
  std::cout << "hello graphics world!" << std::endl;

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
    std::cerr << "Unable to initialize a window!!!" << std::endl;
    return 0;
  }

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
      std::cout << "window resizing..." << std::endl;
    }
  }

  return 0;
}
