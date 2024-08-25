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

#include "GraphicsWindow.h"
#include "graphics/OGLShader.h"
#include "graphics/ShaderProgram.h"

void DrawPrimitive(ShaderProgram & shader_program);
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
                                         "uniform vec4 color; // specify a color output to the fragment shader\n" \
                                         "out vec4 vertexColor; // specify a color output to the fragment shader\n" \
                                         "\n" \
                                         "void main()\n" \
                                         "{\n" \
                                         "    gl_Position = vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor\n" \
                                         "    //vertexColor = vec4(0.5, 0.0, 0.0, 1.0); // set the output variable to a dark-red color\n" \
                                         "    vertexColor = color;\n" \
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

  ShaderProgram shader_program;
  shader_program.AttachShader({vshader, fshader});
  shader_program.Use();

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

    DrawPrimitive(shader_program);

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

void DrawPrimitive(ShaderProgram & shader_programs)
{
  static float vertices[] = {
      // positions        // colors
      0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
     -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
      0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
  };

  static GLuint vao_handle = 0;
  glGenVertexArrays(1, &vao_handle);
  glBindVertexArray(vao_handle);

  static GLuint vbo_handle = 0;
  glGenBuffers(1, &vbo_handle);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);


  static float timeValue = 0.1f;
  timeValue += 0.1f;

  float greenValue = std::sin(timeValue) / 2.0f + 0.5f;
  float redValue = std::cos(timeValue) / 2.0f + 0.5f;
  int vertexColorLocation = glGetUniformLocation(shader_programs.GetProgramId(), "color");
  glUniform4f(vertexColorLocation, redValue, greenValue, 0.0f, 1.0f);

  // now render the triangle
  glDrawArrays(GL_TRIANGLES, 0, 3);
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
