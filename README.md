Неделя 1 до 3: окно, цветной треугольник, шейдеры из файлов.

Управление: Esc выход, пробел белый фон.

Сборка (MinGW, нужны GLFW и GLAD 2):
gcc -c glad/src/gl.c -Iglad/include -o gl.o
g++ main.cpp gl.o -Iglad/include -Iinclude -Llib-mingw-w64 -lglfw3 -lgdi32 -o main.exe

Запускать из папки проекта (рядом vertex.glsl и fragment.glsl).
