cmake_minimum_required(VERSION 3.25)
project (graphics VERSION 0.1.0)

set(CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

## Add SDL library (Simple Direct Media Libary)

set(SDL_STATIC TRUE)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/sdl)

## Add GLAD (Graphics Library Extension Wrangler) For OpenGL Support

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/glad)

## Add ImGUI library

include_directories(${PROJECT_SOURCE_DIR}/libs/gui)
include_directories(${PROJECT_SOURCE_DIR}/libs/gui/imgui)
file(GLOB IMGUI_SRC ${PROJECT_SOURCE_DIR}/libs/gui/imgui/*.cpp)
file(GLOB IMGUI_SRC_OGLSUPPORT ${PROJECT_SOURCE_DIR}/libs/gui/imgui/backends/imgui_impl_opengl3*)
file(GLOB IMGUI_SRC_SDLSUPPORT ${PROJECT_SOURCE_DIR}/libs/gui/imgui/backends/imgui_impl_sdl3*)

## Add SPD Log

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/spdlog)

## Add OpenGL packages
find_package(OpenGL REQUIRED)

## Link and Executable Configuration

file(GLOB GRAPHICS_FILES graphics/*)

add_executable(${PROJECT_NAME}
               ${IMGUI_SRC}
               ${IMGUI_SRC_OGLSUPPORT}
               ${IMGUI_SRC_SDLSUPPORT}
               ${GRAPHICS_FILES}
               ${GLAD_SOURCES}
               GraphicsWindow.h
               GraphicsWindow.cpp
               OpenGLContext.h
               OpenGLContext.cpp
               main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
                      SDL3-static
                      ${OPENGL_LIBRARIES}
                      glad
                      spdlog::spdlog)

target_include_directories(${PROJECT_NAME} PUBLIC
		                   $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:glad,INTERFACE_INCLUDE_DIRECTORIES)

## Compiler Options

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:/W4 /WX /Od>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:/W4 /WX /Ot /O2>)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-Wall -Wextra -Wpedantic -Wunused -Werror -O0 -g>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-Wall -Wextra -Wpedantic -Wunused -Werror -O3>)
endif ()
