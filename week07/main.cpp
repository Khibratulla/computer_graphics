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

void main() {
    FragColor = vec4(faceColor, 1.0);
}
)";

int gWidth = 800, gHeight = 600;

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    gWidth = width;
    gHeight = height;
}

bool ortho = false;
bool threeCubes = false;
int fovIndex = 1, nearIndex = 0, farIndex = 0;
const float fovs[] = {30.0f, 45.0f, 90.0f, 120.0f};
const float nearPlanes[] = {0.1f, 1.0f, 2.5f, 4.0f};
const float farPlanes[] = {100.0f, 5.0f, 2.0f};

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_1) threeCubes = false;
    if (key == GLFW_KEY_3) threeCubes = true;
    if (key == GLFW_KEY_O) ortho = !ortho;
    if (key == GLFW_KEY_F) fovIndex = (fovIndex + 1) % 4;
    if (key == GLFW_KEY_N) nearIndex = (nearIndex + 1) % 4;
    if (key == GLFW_KEY_B) farIndex = (farIndex + 1) % 3;
    if (key == GLFW_KEY_R) {
        fovIndex = 1;
        nearIndex = farIndex = 0;
        ortho = threeCubes = false;
    }
    std::cout << (ortho ? "Orthographic" : "Perspective") << " | FOV " << fovs[fovIndex]
              << " | near " << nearPlanes[nearIndex] << " | far " << farPlanes[farIndex] << std::endl;
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
    GLFWwindow* window = glfwCreateWindow(gWidth, gHeight, "Week 07: MVP and projection", nullptr, nullptr);
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

    const unsigned int shader = createProgram(vertexSrc, fragmentSrc);
    if (!shader) {
        glfwTerminate();
        return 1;
    }
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
        {-1.5f, 0.0f, 0.0f},
        { 0.0f, 0.0f, -1.0f},
        { 1.5f, 0.0f, 0.0f}
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

    std::cout << "1/3: one/three cubes | O: projection | F: FOV | N: near | B: far | R: reset | Esc: exit\n";
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
        glClear(GL_COLOR_BUFFER_BIT);
        const float aspect = static_cast<float>(gWidth) / gHeight;
        const glm::mat4 projection = ortho
            ? glm::ortho(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f)
            : glm::perspective(glm::radians(fovs[fovIndex]), aspect,
                               nearPlanes[nearIndex], farPlanes[farIndex]);
        const glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
                                          glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
        for (int i = 0; i < (threeCubes ? 3 : 1); ++i) {
            const glm::mat4 model = glm::rotate(
                glm::translate(glm::mat4(1.0f), threeCubes ? cubePositions[i] : glm::vec3(0.0f)),
                angle * (0.5f + i * 0.3f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader);
    glfwTerminate();
}
