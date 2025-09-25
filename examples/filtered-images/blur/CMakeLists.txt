cmake_minimum_required(VERSION 3.25)
project (blur-example VERSION 1.0.0)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

## define project root directory to make it easier to define file locations in code related to project

add_definitions(-DEXAMPLES_PROJECT_ROOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../")

# setup application

add_executable(${PROJECT_NAME}
               ${GLAD_SOURCES}
               ${GRAPHICS_FILES}
               main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC graphics-static)

target_link_libraries(${PROJECT_NAME} PUBLIC graphics-static)

target_link_libraries(${PROJECT_NAME} PRIVATE
                      imgui
                      imgui_backend
                      cthreads)

## Compiler Options

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:/W4 /WX /arch:AVX2 /Od>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:/W4 /WX /arch:AVX2 /Ot /O2>)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O0 -g>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O3>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>:-ggdb -Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror -mavx2 -O3>)
endif ()
