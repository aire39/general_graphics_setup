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

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <CLI/CLI.hpp>

#include "GraphicsWindow.h"
#include "graphics/OGLShader.h"
#include "graphics/ShaderProgram.h"
#include "graphics/FImage.h"

#include "graphics/PrebuiltShaderSources.h"
#include "graphics/Filters.h"

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

  // Apply gray scale filter on image

  constexpr bool save_filter = true;
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  sprite.ProcessFilter(filter::functions::cpu::parallel_vectorize::convert_to_grayscale, save_filter);
  sprite.ProcessFilter(filter::functions::cpu::parallel_vectorize::convert_to_grayscale, save_filter);

  sprite.ViewTexture(1);
  sprite.ProcessFilter(filter::functions::cpu::parallel_vectorize::convert_to_grayscale, save_filter);

  sprite.ViewTexture(1);
  sprite.ProcessFilter(filter::functions::cpu::parallel_vectorize::convert_to_grayscale, save_filter);

  sprite.ViewTexture(2);
  sprite.ProcessFilter(filter::functions::cpu::parallel_vectorize::convert_to_grayscale, save_filter);

  sprite.ChangeFilterName("3", "change");

  sprite.ChangeFilterName(4, "change");
  sprite.ChangeFilterName("change", "change");

  std::vector<std::pair<std::string, int>> processed_image_names_0 = sprite.GetProcessedImageNames();
  std::string name_list_0;
  for (const auto & [name, index] : processed_image_names_0 )
  {
    name_list_0 += "[" + name + " (" + std::to_string(index) + ")] ";
  }
  logging::info("processed_image_names image names: {}", name_list_0);

  sprite.ProcessFilter("change", "test", filter::functions::cpu::vectorize::convert_to_grayscale);

  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

  double elapsed_time = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.0;
  logging::info(fmt::format(fmt::fg(fmt::terminal_color::bright_white) | fmt::emphasis::bold, "{}({}) Filter process (par_unseq) elapsed time: {}ms", sprite.GetName(), sprite.GetID(), elapsed_time));

  std::vector<std::pair<std::string, int>> processed_image_names = sprite.GetProcessedImageNames();
  std::string name_list;
  for (const auto & [name, index] : processed_image_names )
  {
    name_list += "[" + name + " (" + std::to_string(index) + ")] ";
  }
  logging::info("processed_image_names image names: {} --> current view: {}", name_list, sprite.GetCurrentImageFilterID());

  auto image = sprite.Extract(5);
  image.reset();

  processed_image_names = sprite.GetProcessedImageNames();
  name_list = "";
  for (const auto & [name, index] : processed_image_names )
  {
    name_list += "[" + name + " (" + std::to_string(index) + ")] ";
  }
  logging::info("processed_image_names image names: {}", name_list);

  // initialize imGUI

  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  constexpr std::string_view glsl_version = "#version 450";
  ImGui_ImplSDL3_InitForOpenGL(graphics_window.GetSDLWindow(), graphics_window.GetOpenGLContext()->GetContext());
  ImGui_ImplOpenGL3_Init(glsl_version.data());

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

    SetupShaderParams(shader_program);

    static float location_time = 1.0f;
    static bool switch_layer = false;
    location_time += 0.05f;

    auto model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, sprite.GetPosition());
    model_matrix = glm::scale(model_matrix, {800.0f, 600.0f, 1.0f});
    shader_program.SetFloat4x4("model", glm::value_ptr(model_matrix));

    if (static_cast<int32_t>(location_time) % 10 == 0)
    {
      location_time = 1.0f;

      if (!switch_layer)
      {
        sprite.ViewTexture(sprite.GetBaseImageFilterID());
        logging::info("switch to base layer!");
      }
      else
      {
        sprite.ViewTexture(sprite.GetFinalImageFilterID());
        logging::info("switch to other layer!");
      }

      switch_layer ^= true;
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