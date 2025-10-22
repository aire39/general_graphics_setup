@echo off
setlocal enabledelayedexpansion

rem ======================================
rem Arguments
rem ======================================
set "BUILD_TYPE=%~1"
set "INSTALL_PREFIX=%~2"
set "PLATFORM=%~3"

rem ======================================
rem Paths
rem ======================================
set "CUR_DIR=%cd%"
set "OPENCV_LOCATION=%CUR_DIR%\libs\opencv"
set "OPENCV_CONTRIB_LOCATION=%CUR_DIR%\libs\opencv_contrib\modules"
set "BUILD_DIR=opencv-%PLATFORM%"

rem ======================================
rem Move into OpenCV source directory
rem ======================================
cd /d "%OPENCV_LOCATION%"
echo Current directory: %cd%

if not exist "%BUILD_DIR%" (
    echo Creating build directory "%BUILD_DIR%"
    mkdir "%BUILD_DIR%"
)
cd "%BUILD_DIR%"

rem ======================================
rem CMake arguments
rem ======================================
set COMMON_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX% -DBUILD_LIST=core,imgproc,imgcodecs,highgui -DCMAKE_CXX_FLAGS="/w /WX-" -DCMAKE_C_FLAGS="/w /WX-" -DBUILD_SHARED_LIBS=ON -DWITH_EIGEN=OFF -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_EXAMPLES=OFF -DCPU_BASELINE_REQUIRE="SSE;SSE2;SSE3;SSSE3;AVX;AVX2" -DOPENCV_EXTRA_MODULES_PATH=%OPENCV_CONTRIB_LOCATION%

rem ======================================
rem Attempt build with Ninja
rem ======================================
echo Running initial CMake configuration with Ninja generator...
cmake -G "Ninja" %COMMON_ARGS% ..
if errorlevel 1 (
    echo CMake configuration failed, retrying with default generator...
    
    rem Clean up cache
    if exist CMakeCache.txt del /q CMakeCache.txt
    if exist CMakeFiles rmdir /s /q CMakeFiles

    rem Retry configuration
    cmake %COMMON_ARGS% ..   
)

cmake --build .

rem ======================================
rem Install
rem ======================================
cmake --install .

echo OpenCV build and install complete!
endlocal
exit /b 0
