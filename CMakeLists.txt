cmake_minimum_required(VERSION 3.25)
project (graphics VERSION 0.2.0)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

## define project root directory to make it easier to define file locations in code related to project

add_definitions(-DPROJECT_ROOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")

## get nasm assembler for windows

set(NASM_VERSION "2.16.03" CACHE STRING "NASM version")
set(NASM_HASH_SHA512 "22869CEB70EA0E6597FE06ABE205B5D5DD66B41FE54DDA73D338C488BA6EF13A39158F25B357616BF578752BB112869EF26AD897EB29352E85CF1ECC61A7C07A" CACHE STRING "SHA512 string for NASM")
if (WIN32)
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

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/sdl_image)

## Add GLAD (Graphics Library Extension Wrangler) For OpenGL Support

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/glad)

## add magic enum (extends to use of enums)

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/magic_enum)

## Add ImGUI library

include_directories(${PROJECT_SOURCE_DIR}/libs/gui)
include_directories(${PROJECT_SOURCE_DIR}/libs/gui/imgui)
file(GLOB IMGUI_SRC ${PROJECT_SOURCE_DIR}/libs/gui/imgui/*.cpp)
file(GLOB IMGUI_SRC_BACKEND ${PROJECT_SOURCE_DIR}/libs/gui/imgui/backends/imgui_impl_opengl3* ${PROJECT_SOURCE_DIR}/libs/gui/imgui/backends/imgui_impl_sdl3*)

add_library(imgui STATIC ${IMGUI_SRC})
add_library(imgui_backend STATIC ${IMGUI_SRC_BACKEND})

target_include_directories(imgui PRIVATE libs/gui/imgui libs/gui/imgui)
target_include_directories(imgui_backend PRIVATE libs/gui/imgui libs/gui/imgui libs/gui/imgui/backends libs/sdl/include)

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

## custom thread code

set(MAX_ENHANCE_SPRITE_THREADS 8 CACHE STRING "Set maximum number of threads for EnhanceSprite threadpool")
add_definitions(-DMAX_ENHANCE_SPRITE_THREADS=${MAX_ENHANCE_SPRITE_THREADS})

file(GLOB CTHREADS_SRC ${PROJECT_SOURCE_DIR}/common/cthreads/*)
add_library(cthreads STATIC ${CTHREADS_SRC})
target_include_directories(cthreads PUBLIC ${PROJECT_SOURCE_DIR}/common)

## Add TBB to enable parallel execution
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
else()
  find_package(TBB REQUIRED)
endif ()

## Link and Executable Configuration

file(GLOB GRAPHICS_FILES graphics/*)

# setup examples

option(BUILD_ALL_EXAMPLES "Build All Examples" OFF)
option(BUILD_EXAMPLE_FIMAGE_GRAYSCALE "Build Grayscale Image Filtering Example" OFF)
option(BUILD_EXAMPLE_FIMAGE_BLUR "Build Blur Image Filtering Example" OFF)
option(BUILD_EXAMPLE_FIMAGE_VARYBITS "Build Varying Bits Image Filtering Example" OFF)
option(BUILD_EXAMPLE_FIMAGE_MANFILT "Build Manual Filtering Example" OFF)

if (BUILD_EXAMPLE_FIMAGE_GRAYSCALE OR BUILD_ALL_EXAMPLES)
    message("Build Grayscale Example...")
    add_subdirectory(examples/filtered-images/grayscale)
endif ()

if (BUILD_EXAMPLE_FIMAGE_BLUR OR BUILD_ALL_EXAMPLES)
    message("Build Blur Example...")
    add_subdirectory(examples/filtered-images/blur)
endif ()

if (BUILD_EXAMPLE_FIMAGE_VARYBITS OR BUILD_ALL_EXAMPLES)
    message("Build Varying Bits Example...")
    add_subdirectory(examples/filtered-images/varyingbits)
endif ()

if (BUILD_EXAMPLE_FIMAGE_MANFILT OR BUILD_ALL_EXAMPLES)
    message("Build Manual Filtering Example...")
    add_subdirectory(examples/filtered-images/manual-filter)
endif ()


# setup application

add_executable(${PROJECT_NAME}
               ${GRAPHICS_FILES}
               ${GLAD_SOURCES}
               GraphicsWindow.h
               GraphicsWindow.cpp
               OpenGLContext.h
               OpenGLContext.cpp
               graphics/FImage.cpp
               graphics/FImage.h
               graphics/PrebuiltShaderSources.h
               graphics/Filters.h
               graphics/Filters.cpp
               graphics/PixelData.h
               common/support/utility.h
               main.cpp)

add_library(${PROJECT_NAME}-static STATIC
            ${GRAPHICS_FILES}
            ${GLAD_SOURCES}
            GraphicsWindow.h
            GraphicsWindow.cpp
            OpenGLContext.h
            OpenGLContext.cpp
            graphics/FImage.cpp
            graphics/FImage.h
            graphics/PrebuiltShaderSources.h
            graphics/Filters.h
            graphics/Filters.cpp
            graphics/PixelData.h
            common/support/utility.h)

target_link_libraries(${PROJECT_NAME} PUBLIC
                      SDL3::SDL3-static
                      SDL3_image::SDL3_image
                      ${OPENGL_LIBRARIES}
                      glad
                      spdlog::spdlog
                      magic_enum::magic_enum
                      glm::glm
                      CLI11::CLI11)

target_link_libraries(${PROJECT_NAME}-static PUBLIC
                      SDL3::SDL3-static
                      SDL3_image::SDL3_image
                      ${OPENGL_LIBRARIES}
                      glad
                      spdlog::spdlog
                      magic_enum::magic_enum
                      glm::glm
                      CLI11::CLI11)

target_link_libraries(${PROJECT_NAME} PRIVATE
                      cthreads)

target_link_libraries(${PROJECT_NAME} PRIVATE
                      imgui
                      imgui_backend
                      cthreads)

target_link_libraries(${PROJECT_NAME}-static PRIVATE
                      imgui
                      imgui_backend
                      cthreads)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC
                        TBB::tbb)

  target_link_libraries(${PROJECT_NAME}-static PUBLIC
          TBB::tbb)
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC
                           $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:SDL3_image::SDL3_image,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:glad,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:magic_enum::magic_enum,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:glm::glm,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:CLI11::CLI11,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:cthreads::cthreads,INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(${PROJECT_NAME}-static PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:SDL3_image::SDL3_image,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:glad,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:magic_enum::magic_enum,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:glm::glm,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:CLI11::CLI11,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:cthreads::cthreads,INTERFACE_INCLUDE_DIRECTORIES)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/glsl
                   ${CMAKE_CURRENT_BINARY_DIR}/glsl)

## Compiler Options

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:/W4 /WX /arch:AVX2 /Od>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:/W4 /WX /arch:AVX2 /Ot /O2>)
else ()
    target_compile_options(${PROJECT_NAME}-static PRIVATE $<$<CONFIG:DEBUG>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O0 -g>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O0 -g>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O3>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>:-ggdb -Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O3>)
endif ()
