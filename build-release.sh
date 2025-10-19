#!/bin/bash

set -e

rm -rf build-release
mkdir build-release
cd build-release

if command -v ninja > /dev/null 2>&1; then
	echo "Using Ninja Generator"
	CMAKE_GEN='-G Ninja'
else
	echo "Using Unix Makefiles Generator"
	CMAKE_GEN='-G Unix Makefiles'
fi

cmake $CMAKE_GEN -DBUILD_ALL_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release ..


if command -v ninja > /dev/null 2>&1; then
	cmake --build . --config=release
else
	cmake --build . --config=release -- -j
fi

cd ..
