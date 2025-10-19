#!/bin/bash

set -e

rm -rf build-debug
mkdir build-debug
cd build-debug

if command -v ninja > /dev/null 2>&1; then
	echo "Using Ninja Generator"
	CMAKE_GEN='-G Ninja'
else
	echo "Using Unix Makefiles Generator"
	CMAKE_GEN='-G Unix Makefiles'
fi

cmake $CMAKE_GEN -DBUILD_ALL_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug ..


if command -v ninja > /dev/null 2>&1; then
	cmake --build . --config=debug
else
	cmake --build . --config=debug -- -j
fi

cd ..
