cmake_minimum_required(VERSION 3.20)
project(week10 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(glm
    URL https://codeload.github.com/g-truc/glm/tar.gz/refs/tags/1.0.1
    URL_HASH SHA256=9F3174561FD26904B23F0DB5E560971CBF9B3CBDA0B280F04D5C379D03BF234C
)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "")
FetchContent_MakeAvailable(glm)

configure_file(../glad/src/gl.c gl.c COPYONLY)
add_executable(week10 main.cpp "${CMAKE_CURRENT_BINARY_DIR}/gl.c")
target_compile_features(week10 PRIVATE cxx_std_17)
target_include_directories(week10 PRIVATE ../glad/include ../include)
if(MSVC)
    # The x64 GLFW DLL and its COFF import archive are already in the repository.
    target_compile_definitions(week10 PRIVATE GLFW_DLL)
    configure_file(../lib-mingw-w64/libglfw3dll.a glfw3.lib COPYONLY)
    target_link_libraries(week10 PRIVATE glm::glm "${CMAKE_CURRENT_BINARY_DIR}/glfw3.lib")
    add_custom_command(TARGET week10 POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../lib-mingw-w64/glfw3.dll"
            "$<TARGET_FILE_DIR:week10>/glfw3.dll"
        VERBATIM)
else()
    # Keep non-ASCII repository paths out of MinGW's linker arguments.
    configure_file(../lib-mingw-w64/libglfw3.a libglfw3.a COPYONLY)
    target_link_libraries(week10 PRIVATE glm::glm -L. glfw3 gdi32)
endif()
