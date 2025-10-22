rem @echo off
CALL "C:\msys64\msys2_shell.cmd" -mingw64 -defterm -no-start -c "cd general_graphics_setup; sh scripts/gcc/build-opencv.sh %*"
