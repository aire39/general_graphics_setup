## fetch opencv

include("${CMAKE_CURRENT_LIST_DIR}/utilities.cmake")
include(FetchContent)

if (EXISTS "${CMAKE_SOURCE_DIR}/libs/opencv_contrib")
else ()
        colored_message("1;34" "Fetch OpenCV contributions...")
        file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/libs/opencv_contrib")
        FetchContent_Populate(
                ocv_contrib
                GIT_REPOSITORY https://github.com/opencv/opencv_contrib.git
                GIT_TAG        4.12.0  # or latest release
        )
file(RENAME "${ocv_contrib_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/libs/opencv_contrib")
        colored_message("1;34" "Fetch OpenCV contributions complete!")
endif ()

if (EXISTS "${CMAKE_SOURCE_DIR}/libs/opencv/")
else ()
        colored_message("1;34" "Fetch OpenCV...")
        file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/libs/opencv")
        FetchContent_Populate(
                        ocv
                        GIT_REPOSITORY https://github.com/opencv/opencv.git
                        GIT_TAG        4.12.0  # or latest release
                )
	file(RENAME "${ocv_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/libs/opencv")
        colored_message("1;34" "Fetch OpenCV complete! -- SOURCE_DIR: ${CMAKE_SOURCE_DIR}/libs/opencv/")
endif ()

## build opencv

# Path where OpenCV will be installed
set(OPENCV_INSTALL_DIR ${CMAKE_BINARY_DIR}/opencv-install CACHE PATH "Path where OpenCV will be installed")

# Path to opencv_contrib modules folder (change if different)
set(OPENCV_CONTRIB_MODULES_DIR ${CMAKE_SOURCE_DIR}/libs/opencv_contrib/modules CACHE PATH "Path to OpenCV contrib modules")

if(MSVC)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv_d-msvc)
        file(GLOB OPENCV_DLLS "${CMAKE_SOURCE_DIR}/opencv_d-msvc/x64/*/bin/*.dll")
        execute_process(
                COMMAND cmd.exe /c "${CMAKE_SOURCE_DIR}/scripts/msvc/build-opencv.bat"
                "${CMAKE_BUILD_TYPE}"
                "${CMAKE_SOURCE_DIR}/opencv_d-msvc"
                "msvc_d"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_VARIABLE errors
        )
    else()
        set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv-msvc)
        file(GLOB OPENCV_DLLS "${CMAKE_SOURCE_DIR}/opencv-msvc/x64/*/bin/*.dll")
        execute_process(
                COMMAND cmd.exe /c "${CMAKE_SOURCE_DIR}/scripts/msvc/build-opencv.bat"
                "${CMAKE_BUILD_TYPE}"
                "${CMAKE_SOURCE_DIR}/opencv-msvc"
                "msvc"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_VARIABLE errors
        )
    endif ()
elseif((UNIX OR RUNNING_ON_WSL) AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv_d-nix-gcc/lib/cmake/opencv4)
        execute_process(
                COMMAND "${CMAKE_SOURCE_DIR}/scripts/gcc/build-opencv.sh"
                "${CMAKE_BUILD_TYPE}"
                "${CMAKE_SOURCE_DIR}/opencv_d-nix-gcc"
                "nix_d"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_VARIABLE errors
        )
    else()
	message("build opencv linux")
        set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv-nix-gcc/lib/cmake/opencv4)
        execute_process(
                COMMAND "${CMAKE_SOURCE_DIR}/scripts/gcc/build-opencv.sh"
                "${CMAKE_BUILD_TYPE}"
                "${CMAKE_SOURCE_DIR}/opencv-nix-gcc"
                "nix"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_VARIABLE errors
        )
		message("result: ${result}")
		message("output: ${output}")
		message("errors: ${errors}")
    endif ()
elseif(WIN32 AND MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv_d-mingw-gcc)
        file(GLOB OPENCV_DLLS "${CMAKE_SOURCE_DIR}/opencv_d-mingw-gcc/x64/mingw/bin/*.dll")
        execute_process(
                COMMAND cmd.exe /c "${CMAKE_SOURCE_DIR}/scripts/gcc/build-opencv.bat"
                "${CMAKE_BUILD_TYPE}"
                "${CMAKE_SOURCE_DIR}/opencv_d-mingw-gcc"
                "mingw_d"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_VARIABLE errors
        )
    else()
        set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv-mingw-gcc)
        file(GLOB OPENCV_DLLS "${CMAKE_SOURCE_DIR}/opencv-mingw-gcc/x64/mingw/bin/*.dll")
        execute_process(
                COMMAND cmd.exe /c "${CMAKE_SOURCE_DIR}/scripts/gcc/build-opencv.bat"
                "${CMAKE_BUILD_TYPE}"
                "${CMAKE_SOURCE_DIR}/opencv-mingw-gcc"
                "mingw"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_VARIABLE errors
        )
    endif ()
elseif(WIN32 AND MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv_d-mingw-clang)
        file(GLOB OPENCV_DLLS "${CMAKE_SOURCE_DIR}/opencv_d-mingw-clang/x64/mingw/bin/*.dll")
        execute_process(
                COMMAND cmd.exe /c "${CMAKE_SOURCE_DIR}/scripts/clang/build-opencv.bat"
                "${CMAKE_BUILD_TYPE}"
                "${CMAKE_SOURCE_DIR}/opencv_d-mingw-clang"
                "clang_d"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_VARIABLE errors
        )
    else()
        set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv-mingw-clang)
        file(GLOB OPENCV_DLLS "${CMAKE_SOURCE_DIR}/opencv-mingw-clang/x64/mingw/bin/*.dll")
        execute_process(
                COMMAND cmd.exe /c "${CMAKE_SOURCE_DIR}/scripts/clang/build-opencv.bat"
                "${CMAKE_BUILD_TYPE}"
                "${CMAKE_SOURCE_DIR}/opencv-mingw-clang"
                "clang"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_VARIABLE errors
        )
    endif ()
endif()
