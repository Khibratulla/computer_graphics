#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

std::string readFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int compileShader(GLenum type, const char* path) {
    std::string code = readFile(path);
    const char* src = code.c_str();

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Ошибка компиляции шейдера (" << path << "):\n" << log << std::endl;
    }
    return shader;
}

unsigned int createProgram(const char* vertPath, const char* fragPath) {
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertPath);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragPath);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Ошибка линковки программы:\n" << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Graphics", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(0);

    double lastTime = glfwGetTime();
    int frames = 0;

    // x, y, r, g, b
    float vertices[] = {
         0.0f,  0.6f,   1.0f, 0.5f, 0.0f,
        -0.6f, -0.5f,   0.0f, 1.0f, 1.0f,
         0.6f, -0.5f,   1.0f, 0.0f, 1.0f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Неделя 3: шейдерная программа из файлов
    unsigned int program = createProgram("vertex.glsl", "fragment.glsl");

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        float t = (float)glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        else
            glClearColor((sin(t) + 1) * 0.15f, (sin(t * 1.5f) + 1) * 0.15f, 0.35f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        frames++;
        if (glfwGetTime() - lastTime >= 1.0) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            lastTime = glfwGetTime();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
    glfwTerminate();
}