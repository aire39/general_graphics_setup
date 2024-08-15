#include "OpenGLContext.h"

#include "GraphicsWindow.h"

#include <iostream>

OpenGLContext::OpenGLContext(GraphicsWindow & graphics_window)
{
  SDL_GL_LoadLibrary(nullptr);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

  context = SDL_GL_CreateContext(graphics_window.GetSDLWindow());
}

OpenGLContext::~OpenGLContext()
{
  std::cout << "Destroy OpenGLContext!" << std::endl;
  SDL_GL_DestroyContext(context);
}

SDL_GLContext OpenGLContext::GetContext() const
{
  return context;
}