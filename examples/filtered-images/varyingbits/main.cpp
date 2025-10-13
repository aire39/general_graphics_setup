#ifdef _WIN32
#include <windows.h>
#endif

#include <cmath>
#include <string>

#include <glad/glad.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <SDL3/SDL.h>

#include "common/support/logging.h"
#include <spdlog/fmt/bundled/color.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <CLI/CLI.hpp>

#include "GraphicsWindow.h"
#include "graphics/shaders/OGLShader.h"
#include "graphics/shaders/ShaderProgram.h"
#include "graphics/images/FImage.h"

#include "graphics/shaders/PrebuiltShaderSources.h"
#include "graphics/filters/FilterTypes.h"
#include "filters/VaryingFilters.h"
#include "gui/VaryingBitsMenu.h"

void SetupShaderParams(const ShaderProgram & shader_program);
bool WindowResize(void * data, SDL_Event * event);
void SetConsoleMode();
void PrintStartMessage();

int32_t main(int32_t argc, char*argv[])
{
  SetConsoleMode();

  CLI::App app("easily setup graphics source for getting started with opengl and potentially other graphic libraries for <reason for template>", "graphics");

  // add command line options,flags,etc

  CLI11_PARSE(app, argc, argv)

  PrintStartMessage();

  // sdl and window initialization

  constexpr static std::string_view window_name = "Graphics Window";
  constexpr int32_t window_width  = 800;
  constexpr int32_t window_height = 600;
  constexpr uint64_t sdl_window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

  auto graphics_window = GraphicsWindow(window_name.data(), window_width, window_height, sdl_window_flags);
  SDL_GL_MakeCurrent(graphics_window.GetSDLWindow(), graphics_window.GetOpenGLContext()->GetContext());

  if (!graphics_window.IsWindowInitialized())
  {
    logging::error("Unable to initialize a window!!!");
    return 0;
  }

  gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress));

  const OGLShader vshader(prebuilt_shaders::vertex_shader_code.data(), OGLShader::ShaderType::VERTEX);
  const OGLShader fshader(prebuilt_shaders::fragment_shader_code.data(), OGLShader::ShaderType::FRAGMENT);
  const ShaderProgram shader_program(vshader, fshader);
  shader_program.Use();

  // Init Sprite with Texture

  FImage sprite;
  sprite.LoadTexture(std::string(EXAMPLES_PROJECT_ROOT_PATH) + "images/park.jpg");
  shader_program.SetTexture2D("image", sprite.GetTexture());

  // Apply varying bits filter on image

  constexpr bool save_filter = true;
  constexpr int32_t repeat_filter_amount = -1;
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

  auto varying_bits_process = filter::functions::cpu::parallel_vectorize::varying_bits_process;
  sprite.ProcessFilter(varying_bits_process, save_filter, repeat_filter_amount);

  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

  double elapsed_time = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.0;
  logging::info(fmt::format(fmt::fg(fmt::terminal_color::bright_white) | fmt::emphasis::bold, "{}({}) Filter process (par_unseq) elapsed time: {}ms", sprite.GetName(), sprite.GetID(), elapsed_time));

  // initialize imGUI

  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  constexpr std::string_view glsl_version = "#version 450";
  ImGui_ImplSDL3_InitForOpenGL(graphics_window.GetSDLWindow(), graphics_window.GetOpenGLContext()->GetContext());
  ImGui_ImplOpenGL3_Init(glsl_version.data());

  VaryingBitsMenu varying_bits_menu;

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
    varying_bits_menu.RenderMenu();

    auto & int_value = std::get<int>(std::get<filter::types::FilterUserTypes>(varying_bits_process));

    if (varying_bits_menu.ShowHidePlaneMode())
    {
      int_value = varying_bits_menu.BitPlaneMask() - 1;
    }
    else
    {
      int_value = (0xFF >> varying_bits_menu.BitScale()) << varying_bits_menu.BitScale();
    }

    const int32_t filter_image_id = sprite.GetFinalImageFilterID();
    constexpr bool no_save_filter = false;
    sprite.ProcessFilter(filter_image_id, varying_bits_process, no_save_filter);

    ImGui::Render();
    GraphicsWindow::ClearWindow();

    // do work

    SetupShaderParams(shader_program);

    static float location_time = 1.0f;
    static bool switch_layer = false;
    location_time += 0.05f;

    auto model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, sprite.GetPosition());
    model_matrix = glm::scale(model_matrix, {800.0f, 600.0f, 1.0f});
    shader_program.SetFloat4x4("model", glm::value_ptr(model_matrix));

    if (location_time > 10.0f)
    {
      location_time = 0.0f;
      switch_layer ^= true;

      if (!switch_layer)
      {
        logging::info("switch to base layer!");
        sprite.ViewTexture(sprite.GetBaseImageFilterID());
      }
      else
      {
        logging::info("switch to other layer!");
      }
    }

    if (switch_layer)
    {
      sprite.ViewTexture(sprite.GetFinalImageFilterID());
    }

    sprite.Draw();

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

void SetupShaderParams(const ShaderProgram & shader_program)
{
  constexpr float l = -400;
  constexpr float r =  400;
  constexpr float b =  300;
  constexpr float t = -300;
  constexpr float n = -1.0f;
  constexpr float f =  1.0f;

  auto ortho_matrix = glm::ortho(l, r, b, t, n, f);
  shader_program.SetFloat4x4("ortho", glm::value_ptr(ortho_matrix));

  static float timeValue = 0.01f;
  timeValue += 0.05f;

  const float green_value = std::sin(timeValue) / 2.0f + 0.5f;
  const float red_value = std::cos(timeValue) / 2.0f + 0.5f;
  const float blue_value = std::sin(timeValue) / 3.0f + 0.5f;
  const float color[] = {red_value, green_value, blue_value};

  shader_program.SetFloat4("color", color);
}

bool WindowResize(void * data, SDL_Event * event)
{
  bool event_handled = false;

  if (event->window.type == SDL_EVENT_WINDOW_RESIZED)
  {
    const SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);
    if (window == static_cast<SDL_Window*>(data))
    {
      logging::info(fmt::format(fmt::fg(fmt::terminal_color::bright_white) | fmt::emphasis::bold, "window resizing..."));
      event_handled = true;
    }
  }

  return event_handled;
}

void SetConsoleMode()
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
}

void PrintStartMessage()
{
  logging::info(
    fmt::format(fmt::fg(fmt::terminal_color::bright_white) | fmt::emphasis::bold
                   ,"Starting: Hello, Graphics World!"));
}