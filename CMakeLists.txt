cmake_minimum_required(VERSION 3.25)
project (graphics VERSION 0.1.0)

set(CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

## Add SDL library (Simple Direct Media Libary)

set(SDL_STATIC TRUE)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/sdl)

## Add ImGUI library

include_directories(${PROJECT_SOURCE_DIR}/libs/gui)
include_directories(${PROJECT_SOURCE_DIR}/libs/gui/imgui)
file(GLOB IMGUI_SRC ${PROJECT_SOURCE_DIR}/libs/gui/imgui/*.cpp)
file(GLOB IMGUI_SRC_OGLSUPPORT ${PROJECT_SOURCE_DIR}/libs/gui/imgui/backends/imgui_impl_opengl3*)
file(GLOB IMGUI_SRC_SDLSUPPORT ${PROJECT_SOURCE_DIR}/libs/gui/imgui/backends/imgui_impl_sdl3*)

## Add OpenGL packages
find_package(OpenGL REQUIRED)

## Link and Executable Configuration

add_executable(${PROJECT_NAME}
               ${IMGUI_SRC}
               ${IMGUI_SRC_OGLSUPPORT}
               ${IMGUI_SRC_SDLSUPPORT}
               GraphicsWindow.h
               GraphicsWindow.cpp
               OpenGLContext.h
               OpenGLContext.cpp
               main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
                      SDL3-static
                      ${OPENGL_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC
		                   $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>)

## Compiler Options

if (MSVC)
  if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /Od)
  else()
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /O2)
  endif()
else ()
  if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wunused -Werror -O0 -g)
  else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wunused -Werror -O3)
  endif()
endif ()
