cmake_minimum_required(VERSION 3.20)
project(week12 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(glm
    URL https://codeload.github.com/g-truc/glm/tar.gz/refs/tags/1.0.1
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL_HASH SHA256=9F3174561FD26904B23F0DB5E560971CBF9B3CBDA0B280F04D5C379D03BF234C
)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "")
FetchContent_MakeAvailable(glm)

configure_file(../glad/src/gl.c gl.c COPYONLY)
add_executable(week12 main.cpp "${CMAKE_CURRENT_BINARY_DIR}/gl.c")
target_compile_features(week12 PRIVATE cxx_std_17)
target_include_directories(week12 PRIVATE ../glad/include ../include ../external/stb)
target_compile_definitions(week12 PRIVATE ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(MSVC)
    target_compile_options(week12 PRIVATE /utf-8)
    # The x64 GLFW DLL and its COFF import archive are already in the repository.
    target_compile_definitions(week12 PRIVATE GLFW_DLL)
    configure_file(../lib-mingw-w64/libglfw3dll.a glfw3.lib COPYONLY)
    target_link_libraries(week12 PRIVATE glm::glm "${CMAKE_CURRENT_BINARY_DIR}/glfw3.lib")
    add_custom_command(TARGET week12 POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../lib-mingw-w64/glfw3.dll"
            "$<TARGET_FILE_DIR:week12>/glfw3.dll"
        VERBATIM)
else()
    # Keep non-ASCII repository paths out of MinGW's linker arguments.
    configure_file(../lib-mingw-w64/libglfw3.a libglfw3.a COPYONLY)
    target_link_libraries(week12 PRIVATE glm::glm -L. glfw3 gdi32)
endif()
