#!/bin/bash

mkdir build-release
cd build-release
cmake -DBUILD_ALL_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config=release
cd ..
