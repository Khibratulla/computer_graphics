@echo off
REM Usage: build.bat week03   (outputs <week>\main.exe)
if "%1"=="" (echo Usage: build.bat ^<week_folder^> & exit /b 1)
if not exist gl.o gcc -c glad/src/gl.c -Iglad/include -o gl.o
g++ -g %1\main.cpp gl.o -Iglad/include -Iinclude -Llib-mingw-w64 -lglfw3 -lgdi32 -o %1\main.exe
copy /y lib-mingw-w64\glfw3.dll %1\ >nul
echo Built %1\main.exe - run it from inside %1 so shaders are found.
