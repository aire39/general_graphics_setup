cmake_minimum_required(VERSION 3.22)
project (graphics VERSION 0.2.0)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

# Add the path to your cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "/opt/ros/humble/share/")

# cmake helper functions

include(cmake/utilities.cmake)

# Detect if running under WSL

set(RUNNING_ON_WSL FALSE)

if(UNIX AND EXISTS "/proc/version")
    file(READ "/proc/version" PROC_VERSION_CONTENTS)
    string(TOLOWER "${PROC_VERSION_CONTENTS}" PROC_VERSION_CONTENTS)  # normalize casing

    if(PROC_VERSION_CONTENTS MATCHES "microsoft-standard"
        OR PROC_VERSION_CONTENTS MATCHES "wsl"
        OR PROC_VERSION_CONTENTS MATCHES "microsoft")
        set(RUNNING_ON_WSL TRUE)
        message(STATUS "Detected WSL environment.")
    endif()
endif()

# Add OpenCV

set(ENABLE_OPENCV_SUPPORT OFF CACHE BOOL "Enable Opencv Support")

if (ENABLE_OPENCV_SUPPORT)
    include(cmake/opencv.cmake)
    find_package(OpenCV REQUIRED)
endif ()

## basic includes

include_directories(${CMAKE_SOURCE_DIR})

## define project root directory to make it easier to define file locations in code related to project

add_definitions(-DPROJECT_ROOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")

set(ENABLE_PLATFORM_CHECKS OFF CACHE BOOL "Enable platform specific checks/fixes")

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

# Add ranges support for clang

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0) )
    include(cmake/ranges-v3.cmake)
    add_range_v3()
endif ()

# Add ranges support for clang

## Add TBB to enable parallel execution
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
else()
  find_package(TBB QUIET)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
else ()
    if (TBB_FOUND)
    else()
        include(cmake/tbb.cmake)
        add_tbb()
    endif()
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

add_compile_definitions(SPDLOG_ENABLE_SOURCE_LOCATION)
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
add_compile_definitions(MAX_ENHANCE_SPRITE_THREADS=${MAX_ENHANCE_SPRITE_THREADS})

file(GLOB CTHREADS_SRC ${PROJECT_SOURCE_DIR}/common/cthreads/*)
add_library(cthreads STATIC ${CTHREADS_SRC})
target_include_directories(cthreads PUBLIC ${PROJECT_SOURCE_DIR}/common)

# ---- Find ROS 2 packages you need ----
if (RUNNING_ON_WSL)
    find_package(ament_cmake REQUIRED)
    find_package(ament_cmake REQUIRED)
    find_package(rclcpp REQUIRED)
    find_package(sensor_msgs REQUIRED)
    find_package(geometry_msgs REQUIRED)
    find_package(gazebo_msgs REQUIRED)
    #find_package(cv_bridge REQUIRED)
endif ()

## Link and Executable Configuration

file(GLOB GRAPHICS_FILTER_FILES graphics/filters/*)
file(GLOB GRAPHICS_GEOMETRY_FILES graphics/geometry/*)
file(GLOB GRAPHICS_IMAGES_FILES graphics/images/*)
file(GLOB GRAPHICS_SHADERS_FILES graphics/shaders/*)
file(GLOB GRAPHICS_OP_FILES graphics/operations/*)
file(GLOB IMAGE_BLOCK_PROCESS_FILES process/*)

if (WIN32 AND NOT RUNNING_ON_WSL)
    unset(CAMERA_SRC)
    file (GLOB CAMERA_SRC video/camera/common/CameraBase.h video/camera/platform/WinCamera.*)
    colored_message("1;34" "Camera support: Windows")
elseif (RUNNING_ON_WSL OR UNIX)
    unset(CAMERA_SRC)
    file (GLOB CAMERA_SRC video/camera/common/CameraBase.h video/camera/platform/LinuxCamera*)
    file (GLOB ROS_CAMERA_SRC video/RosVideoFeed*)
    colored_message("1;34" "Camera support: Linux")
else ()
        colored_message("1;33" "Camera support: None")
        unset(CAMERA_SRC)
endif ()

# grab all gui menus

file (GLOB GUIMENU gui/*)

# setup examples

include(cmake/examples.cmake)

# setup application

add_executable(${PROJECT_NAME}
               ${GRAPHICS_FILTER_FILES}
               ${GRAPHICS_GEOMETRY_FILES}
               ${GRAPHICS_IMAGES_FILES}
               ${GRAPHICS_SHADERS_FILES}
               ${GRAPHICS_OP_FILES}
               ${IMAGE_BLOCK_PROCESS_FILES}
               ${GLAD_SOURCES}
               ${CAMERA_SRC}
               ${ROS_CAMERA_SRC}
               ${GUIMENU}
               GraphicsWindow.h
               GraphicsWindow.cpp
               OpenGLContext.h
               OpenGLContext.cpp
               common/support/utility.h
               main.cpp)

add_library(${PROJECT_NAME}-static STATIC
            ${GRAPHICS_FILTER_FILES}
            ${GRAPHICS_GEOMETRY_FILES}
            ${GRAPHICS_IMAGES_FILES}
            ${GRAPHICS_SHADERS_FILES}
            ${GRAPHICS_OP_FILES}
            ${IMAGE_BLOCK_PROCESS_FILES}
            ${GLAD_SOURCES}
            ${CAMERA_SRC}
            ${GUIMENU}
            GraphicsWindow.h
            GraphicsWindow.cpp
            OpenGLContext.h
            OpenGLContext.cpp
            common/support/utility.h)

if (WIN32)
target_link_libraries(${PROJECT_NAME} PUBLIC
                      SDL3::SDL3-static
                      SDL3_image::SDL3_image
                      ${OPENGL_LIBRARIES}
                      glad
                      spdlog::spdlog
                      magic_enum::magic_enum
                      glm::glm
                      CLI11::CLI11
                      mfplat
                      mfreadwrite
                      mf
                      mfuuid
                      ole32)

target_link_libraries(${PROJECT_NAME}-static PUBLIC
                      SDL3::SDL3-static
                      SDL3_image::SDL3_image
                      ${OPENGL_LIBRARIES}
                      glad
                      spdlog::spdlog
                      magic_enum::magic_enum
                      glm::glm
                      CLI11::CLI11
                      mfplat
                      mfreadwrite
                      mf
                      mfuuid
                      ole32)
else ()

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

ament_target_dependencies(${PROJECT_NAME} PUBLIC
                          rclcpp
                          sensor_msgs
                          geometry_msgs
                          gazebo_msgs
                          #cv_bridge
)

endif ()


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

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})

target_link_libraries(${PROJECT_NAME}-static PRIVATE ${OpenCV_LIBS})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
else()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0))
      target_link_libraries(${PROJECT_NAME} PUBLIC
                            TBB::tbb
                            range-v3)

      target_link_libraries(${PROJECT_NAME}-static PUBLIC
                            TBB::tbb
                            range-v3)
  else ()
      target_link_libraries(${PROJECT_NAME} PUBLIC
                            TBB::tbb)

      target_link_libraries(${PROJECT_NAME}-static PUBLIC
              TBB::tbb)
  endif ()
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC
                           $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:SDL3_image::SDL3_image,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:glad,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:magic_enum::magic_enum,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:glm::glm,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:CLI11::CLI11,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:cthreads::cthreads,INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME}-static PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:SDL3_image::SDL3_image,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:glad,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:magic_enum::magic_enum,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:glm::glm,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:CLI11::CLI11,INTERFACE_INCLUDE_DIRECTORIES
                           $<TARGET_PROPERTY:cthreads::cthreads,INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(${PROJECT_NAME}-static SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/glsl
                   ${CMAKE_CURRENT_BINARY_DIR}/glsl)

foreach(dll ${OPENCV_DLLS})

    if(CMAKE_GENERATOR MATCHES "Visual Studio")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${dll}
                "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
        message("copy dll: ${dll} to ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    else ()
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${dll}
                ${CMAKE_BINARY_DIR})
        message("copy dll: ${dll} to ${CMAKE_BINARY_DIR}")
    endif ()
endforeach()

## Compiler Options

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    if (ENABLE_PLATFORM_CHECKS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_WINDOWS_PLATFORM_CHECKS=1)
    else ()
        target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_WINDOWS_PLATFORM_CHECKS=0)
    endif()

    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:/W4 /WX /arch:AVX2 /Od>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:/W4 /WX /arch:AVX2 /Ot /O2>)
else ()

    if (ENABLE_PLATFORM_CHECKS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_LINUX_PLATFORM_CHECKS=1)
    else ()
        target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_LINUX_PLATFORM_CHECKS=0)
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${PROJECT_NAME}-static PRIVATE $<$<CONFIG:DEBUG>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -Wno-deprecated-declarations -Wno-sign-conversion -Wno-deprecated-literal-operator -mavx2 -O0 -g>)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -Wno-deprecated-declarations -Wno-sign-conversion -Wno-deprecated-literal-operator -mavx2 -O0 -g>)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -Wno-deprecated-declarations -Wno-sign-conversion -Wno-deprecated-literal-operator -mavx2 -O3>)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>:-ggdb -Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -Wno-deprecated-declarations -Wno-deprecated-literal-operator -Wno-sign-conversion -mavx2 -O3>)
    else ()
        target_compile_options(${PROJECT_NAME}-static PRIVATE $<$<CONFIG:DEBUG>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O0 -g>)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O0 -g>)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O3>)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>:-ggdb -Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O3>)
    endif ()
endif ()
