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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <CLI/CLI.hpp>

#include "GraphicsWindow.h"
#include "graphics/OGLShader.h"
#include "graphics/ShaderProgram.h"

#include "graphics/PixelSprite.h"

void SetupShaderParams(ShaderProgram & shader_programs);
int32_t WindowResize(void * data, SDL_Event * event);
void SetConsoleMode();

int32_t main(int32_t argc, char*argv[])
{
  SetConsoleMode();

  CLI::App app("easily setup graphics source for getting started with opengl and potentially other graphic libraries for <reason for template>", "graphics");

  // add command line options,flags,etc

  CLI11_PARSE(app, argc, argv)

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
  constexpr std::string_view vertex_shader_code = "#version 330 core\n" \
                                                  "layout (location = 0) in vec3 aPos; // the position variable has attribute position 0\n" \
                                                  "layout (location = 1) in vec3 aCol; // the color variable has attribute position 1\n" \
                                                  "layout (location = 2) in vec3 aUV0; // the texoord0 variable has attribute position 2\n" \
                                                  "layout (location = 3) in vec3 aUV1; // the texoord1 variable has attribute position 3\n" \
                                                  "\n" \
                                                  "uniform vec4 color; // specify a color output to the fragment shader\n" \
                                                  "uniform mat4 ortho; // specify 2d view matrix\n" \
                                                  "uniform mat4 model; // specify object location matrix\n" \
                                                  "\n" \
                                                  "out vec4 vertexColor; // specify a color output to the fragment shader\n" \
                                                  "out vec4 col; // specify a color output to the fragment shader\n" \
                                                  "\n" \
                                                  "void main()\n" \
                                                  "{\n" \
                                                  "    gl_Position = ortho * model * vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor\n" \
                                                  "    vertexColor = color;\n" \
                                                  "    col = vec4(aCol, 1.0);\n" \
                                                  "}";

  OGLShader vshader(vertex_shader_code.data(), OGLShader::ShaderType::VERTEX);
  //OGLShader vshader("glsl/simple.vert"); // example of loading vertex shader file

  constexpr std::string_view fragment_shader_code = "#version 330 core\n" \
                                                    "out vec4 FragColor;\n" \
                                                    "  \n" \
                                                    "in vec4 col;\n" \
                                                    "in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  \n" \
                                                    "\n" \
                                                    "void main()\n" \
                                                    "{\n" \
                                                    "    FragColor = vertexColor;\n" \
                                                    "}";

  OGLShader fshader(fragment_shader_code.data(), OGLShader::ShaderType::FRAGMENT);
  //OGLShader fshader("glsl/simple.frag"); // example of loading fragment/pixel shader file

  ShaderProgram shader_program;
  shader_program.AttachShader({vshader, fshader});
  shader_program.Use();

  // Init Sprite
  PixelSprite sprite;

  // initialize imGUI
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  const char * glsl_version = "#version 330";
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

    SetupShaderParams(shader_program);

    static float location_time = 0.0f;
    sprite.SetPosition({100.0f * std::cos(location_time), 100.0f * std::sin(location_time)});
    location_time += 0.05f;

    auto model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, sprite.GetPosition());
    model_matrix = glm::rotate(model_matrix, glm::radians(location_time * -100.0f), {0.0f, 0.0f, 1.0f});
    model_matrix = glm::scale(model_matrix, {100.0f, 100.0f, 1.0f});
    shader_program.SetFloat4x4("model", glm::value_ptr(model_matrix));

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

void SetupShaderParams(ShaderProgram & shader_program)
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

  float green_value = std::sin(timeValue) / 2.0f + 0.5f;
  float red_value = std::cos(timeValue) / 2.0f + 0.5f;
  float blue_value = std::sin(timeValue) / 3.0f + 0.5f;
  float color[] = {red_value, green_value, blue_value};
  shader_program.SetFloat4("color", color);
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