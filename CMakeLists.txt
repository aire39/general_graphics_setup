cmake_minimum_required(VERSION 3.25)
project (graphics VERSION 0.1.0)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

## get nasm assembler for windows

set(NASM_VERSION ON CACHE STRING "2.16.03")
set(NASM_HASH_SHA512 ON CACHE STRING "22869ceb70ea0e6597fe06abe205b5d5dd66b41fe54dda73d338c488ba6ef13a39158f25b357616bf578752bb112869ef26ad897eb29352e85cf1ecc61a7c07a")
if (CMAKE_HOST_WIN32)
  include(FetchContent)
  message("-- Fetch NASM Assembler")
  if (EXISTS ${PROJECT_SOURCE_DIR}/nasm)
      message("-- NASM Assembler Already Exists!")
  else ()
      set(FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/nasm")
      FetchContent_Declare(NASM
              URL https://www.nasm.us/pub/nasm/releasebuilds/${NASM_VERSION}/win64/nasm-${NASM_VERSION}-win64.zip
              URL_HASH SHA512=${NASM_HASH_SHA512}
              PREFIX ""
              CONFIGURE_COMMAND ""
              BUILD_COMMAND ""
      )

      FetchContent_MakeAvailable(NASM)
      message("-- NASM Assembler Fetched!")
  endif ()


  set(ASM_NASM ${PROJECT_SOURCE_DIR}/nasm/nasm-src)
  set(CMAKE_ASM_NASM_COMPILER ${PROJECT_SOURCE_DIR}/nasm/nasm-src/nasm.exe)
  set(ENV{PATH} "${PROJECT_SOURCE_DIR}/nasm/nasm-src/;$ENV{PATH}")
  set(NASM_PATH ${PROJECT_SOURCE_DIR}/nasm/nasm-src/nasm.exe)
endif ()

## Add SDL library (Simple Direct Media Libary)

set(SDL_STATIC TRUE)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/sdl)

## Add SDL_Image library (Simple Direct Media Libary)

set(BUILD_SHARED_LIBS OFF)
set(AOM_TARGET_CPU x86_64)

# add missing stdbool win32 patch
if (CMAKE_HOST_WIN32)
    find_program(FOUND_GIT "git")

    if (FOUND_GIT)
        message("-- SDL_image patch: apply missing stdbool")
        execute_process(
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/libs/sdl_image"
                COMMAND git apply ${PROJECT_SOURCE_DIR}/libs/patches/sdl_image_win32_missing_stdbool_fix.patch
        )
    else()
        message("-- Unable to find git command! Will not try to apply patches.")
    endif()
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/sdl_image)

## Add GLAD (Graphics Library Extension Wrangler) For OpenGL Support

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/glad)

## add magic enum (extends to use of enums)

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/magic_enum)

## Add ImGUI library

include_directories(${PROJECT_SOURCE_DIR}/libs/gui)
include_directories(${PROJECT_SOURCE_DIR}/libs/gui/imgui)
file(GLOB IMGUI_SRC ${PROJECT_SOURCE_DIR}/libs/gui/imgui/*.cpp)
file(GLOB IMGUI_SRC_OGLSUPPORT ${PROJECT_SOURCE_DIR}/libs/gui/imgui/backends/imgui_impl_opengl3*)
file(GLOB IMGUI_SRC_SDLSUPPORT ${PROJECT_SOURCE_DIR}/libs/gui/imgui/backends/imgui_impl_sdl3*)

## Add SPD Log

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/spdlog)

## Add GLM Math Library

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/glm)

## Add CLI11 Command Line Parsing Library

set (CLI11_BUILD_DOCS OFF)
set (CLI11_BUILD_EXAMPLES OFF)
set (CLI11_BUILD_EXAMPLES_JSON OFF)
set (CLI11_BUILD_TESTS OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/cli11)

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
               graphics/PixelSprite.cpp
               graphics/PixelSprite.h
               main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
                      SDL3::SDL3-static
                      SDL3_image::SDL3_image
                      ${OPENGL_LIBRARIES}
                      glad
                      spdlog::spdlog
                      magic_enum::magic_enum
                      glm::glm
                      CLI11::CLI11)

target_include_directories(${PROJECT_NAME} PUBLIC
                           $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:SDL3_image::SDL3_image,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:glad,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:magic_enum::magic_enum,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:glm::glm,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:CLI11::CLI11,INTERFACE_INCLUDE_DIRECTORIES)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/glsl
                   ${CMAKE_CURRENT_BINARY_DIR}/glsl)

## Compiler Options

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:/W4 /WX /Od>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:/W4 /WX /Ot /O2>)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-Wall -Wextra -Wpedantic -Wunused -Werror -O0 -g>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-Wall -Wextra -Wpedantic -Wunused -Werror -O3>)
endif ()
