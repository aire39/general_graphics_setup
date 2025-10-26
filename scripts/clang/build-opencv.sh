#!/bin/bash

opencv_location="$(pwd)/libs/opencv"
opencv_contrib_location="$(pwd)/libs/opencv_contrib/modules"

platform=$3

# args
cmake_args=(
  -DCMAKE_BUILD_TYPE="$1"
  -DCMAKE_INSTALL_PREFIX="$2"
  -DBUILD_LIST=core,imgproc,imgcodecs,highgui
  -DCMAKE_CXX_FLAGS="-w -Wno-error"
  -DCMAKE_C_FLAGS="-w -Wno-error"
  -DBUILD_SHARED_LIBS=ON
  -DWITH_EIGEN=OFF
  -DBUILD_TESTS=OFF
  -DBUILD_PERF_TESTS=OFF
  -DBUILD_EXAMPLES=OFF
  -DCPU_BASELINE_REQUIRE="SSE;SSE2;SSE3;SSSE3;AVX;AVX2"
  -DOPENCV_EXTRA_MODULES_PATH="$opencv_contrib_location"
)

# build

cd "$opencv_location" || exit

build_directory="opencv-$platform"

if [ ! -d "${build_directory}" ]; then
  mkdir "opencv-$platform"
fi

cd "opencv-$platform" || exit

cmake -G "Ninja" "${cmake_args[@]}" ..

if [ $? -ne 0 ]; then
    echo "CMake configuration failed"
    
    rm -rf ./.*
    cmake "${cmake_args[@]}" ..
    cmake --build . -- -j
else
    cmake --build .
fi

cmake --install .
