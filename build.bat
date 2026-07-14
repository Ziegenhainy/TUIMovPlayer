mkdir build
cl /std:c11 main.c /Fe: build/tuimovplayer /Fo:build/
.\build\tuimovplayer.exe ./examples/test.tuimov