@echo off

title Build GGS

mkdir build-debug
cd build-debug
cmake -DBUILD_ALL_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config=debug
cd ..
pause