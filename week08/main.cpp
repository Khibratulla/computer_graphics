#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out vec3 faceColor;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    faceColor = aColor;
}
)";

const char* fragmentSrc = R"(
#version 330 core
in vec3 faceColor;
out vec4 FragColor;
uniform vec3 uTint;

void main() {
    FragColor = vec4(faceColor * uTint, 1.0);
}
)";

int gWidth = 800, gHeight = 600;

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    gWidth = width;
    gHeight = height;
}

bool depthEnabled = true;
bool cullFront = false;
int scene = 1;

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_3) scene = key - GLFW_KEY_0;
    if (key == GLFW_KEY_D) {
        depthEnabled = !depthEnabled;
        if (depthEnabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
    }
    if (key == GLFW_KEY_C) {
        cullFront = !cullFront;
        glCullFace(cullFront ? GL_FRONT : GL_BACK);
    }
}

unsigned int compileShader(GLenum type, const char* src) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    if (!ok) { glDeleteShader(shader); return 0; }
    return shader;
}

unsigned int createProgram(const char* vsSrc, const char* fsSrc) {
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);

    if (!vs || !fs) {
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Program link error:\n" << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!ok) { glDeleteProgram(program); return 0; }
    return program;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed.\n";
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(gWidth, gHeight, "Week 08: Depth and culling", nullptr, nullptr);
    if (!window) {
        std::cerr << "OpenGL window creation failed.\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "OpenGL loading failed.\n";
        glfwTerminate();
        return 1;
    }
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwGetFramebufferSize(window, &gWidth, &gHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    const unsigned int shader = createProgram(vertexSrc, fragmentSrc);
    if (!shader) {
        glfwTerminate();
        return 1;
    }
    const int locTint = glGetUniformLocation(shader, "uTint");
    const int locModel = glGetUniformLocation(shader, "uModel");
    const int locView = glGetUniformLocation(shader, "uView");
    const int locProjection = glGetUniformLocation(shader, "uProjection");

    // Six faces, two CCW triangles per face, position + RGB.
    const float vertices[] = {
        -0.50f, -0.50f, -0.50f, 1.00f, 0.24f, 0.25f,
        0.50f, 0.50f, -0.50f, 1.00f, 0.24f, 0.25f,
        0.50f, -0.50f, -0.50f, 1.00f, 0.24f, 0.25f,
        -0.50f, -0.50f, -0.50f, 1.00f, 0.24f, 0.25f,
        -0.50f, 0.50f, -0.50f, 1.00f, 0.24f, 0.25f,
        0.50f, 0.50f, -0.50f, 1.00f, 0.24f, 0.25f,
        -0.50f, -0.50f, 0.50f, 0.20f, 0.68f, 1.00f,
        0.50f, -0.50f, 0.50f, 0.20f, 0.68f, 1.00f,
        0.50f, 0.50f, 0.50f, 0.20f, 0.68f, 1.00f,
        -0.50f, -0.50f, 0.50f, 0.20f, 0.68f, 1.00f,
        0.50f, 0.50f, 0.50f, 0.20f, 0.68f, 1.00f,
        -0.50f, 0.50f, 0.50f, 0.20f, 0.68f, 1.00f,
        -0.50f, -0.50f, -0.50f, 0.18f, 0.85f, 0.47f,
        -0.50f, -0.50f, 0.50f, 0.18f, 0.85f, 0.47f,
        -0.50f, 0.50f, 0.50f, 0.18f, 0.85f, 0.47f,
        -0.50f, -0.50f, -0.50f, 0.18f, 0.85f, 0.47f,
        -0.50f, 0.50f, 0.50f, 0.18f, 0.85f, 0.47f,
        -0.50f, 0.50f, -0.50f, 0.18f, 0.85f, 0.47f,
        0.50f, -0.50f, -0.50f, 1.00f, 0.65f, 0.20f,
        0.50f, 0.50f, -0.50f, 1.00f, 0.65f, 0.20f,
        0.50f, 0.50f, 0.50f, 1.00f, 0.65f, 0.20f,
        0.50f, -0.50f, -0.50f, 1.00f, 0.65f, 0.20f,
        0.50f, 0.50f, 0.50f, 1.00f, 0.65f, 0.20f,
        0.50f, -0.50f, 0.50f, 1.00f, 0.65f, 0.20f,
        -0.50f, -0.50f, -0.50f, 0.65f, 0.38f, 1.00f,
        0.50f, -0.50f, -0.50f, 0.65f, 0.38f, 1.00f,
        0.50f, -0.50f, 0.50f, 0.65f, 0.38f, 1.00f,
        -0.50f, -0.50f, -0.50f, 0.65f, 0.38f, 1.00f,
        0.50f, -0.50f, 0.50f, 0.65f, 0.38f, 1.00f,
        -0.50f, -0.50f, 0.50f, 0.65f, 0.38f, 1.00f,
        -0.50f, 0.50f, -0.50f, 1.00f, 0.87f, 0.30f,
        -0.50f, 0.50f, 0.50f, 1.00f, 0.87f, 0.30f,
        0.50f, 0.50f, 0.50f, 1.00f, 0.87f, 0.30f,
        -0.50f, 0.50f, -0.50f, 1.00f, 0.87f, 0.30f,
        0.50f, 0.50f, 0.50f, 1.00f, 0.87f, 0.30f,
        0.50f, 0.50f, -0.50f, 1.00f, 0.87f, 0.30f,
    };
    const glm::vec3 cubePositions[] = {
        { 0.0f,  0.0f,  0.0f},
        { 2.0f,  0.5f, -2.0f},
        {-1.8f, -0.3f, -1.0f},
        { 1.2f, -0.8f, -3.5f},
        {-2.2f,  1.0f, -4.0f}
    };

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glUseProgram(shader);
    glClearColor(0.08f, 0.09f, 0.13f, 1.0f);

    std::cout << "1: five cubes | 2: Z-fighting | 3: corrected near | D: depth test | C: cull front/back | Esc: exit\n";
    double lastTime = glfwGetTime();
    float angle = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - lastTime);
        lastTime = now;
        angle += dt;

        if (gWidth == 0 || gHeight == 0) continue;
        glViewport(0, 0, gWidth, gHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            static_cast<float>(gWidth) / gHeight,
            scene == 2 ? 0.0001f : 0.1f, scene == 2 ? 1000.0f : 100.0f);
        const glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
                                          glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
        for (int i = 0; i < (scene == 1 ? 5 : 2); ++i) {
            const glm::vec3 position = scene == 1 ? cubePositions[i] : glm::vec3(0.0f, 0.0f, -3.0f - (1 - i) * 0.001f);
            const glm::mat4 model = glm::rotate(
                glm::translate(glm::mat4(1.0f), position),
                scene == 1 ? angle * (0.5f + i * 0.3f) : angle * 0.5f, glm::vec3(0.3f, 1.0f, 0.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            const bool tint = scene != 1 && i == 0;
            glUniform3f(locTint, 1.0f, tint ? 0.35f : 1.0f, tint ? 0.35f : 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader);
    glfwTerminate();
}
