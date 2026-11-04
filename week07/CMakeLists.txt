cmake_minimum_required(VERSION 3.20)
project(week07 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(glm
    URL https://codeload.github.com/g-truc/glm/tar.gz/refs/tags/1.0.1
    URL_HASH SHA256=9F3174561FD26904B23F0DB5E560971CBF9B3CBDA0B280F04D5C379D03BF234C
)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "")
FetchContent_MakeAvailable(glm)

configure_file(../glad/src/gl.c gl.c COPYONLY)
add_executable(week07 main.cpp "${CMAKE_CURRENT_BINARY_DIR}/gl.c")
target_compile_features(week07 PRIVATE cxx_std_17)
target_include_directories(week07 PRIVATE ../glad/include ../include)
# Keep non-ASCII repository paths out of MinGW's linker arguments.
configure_file(../lib-mingw-w64/libglfw3.a libglfw3.a COPYONLY)
target_link_libraries(week07 PRIVATE glm::glm -L. glfw3 gdi32)
