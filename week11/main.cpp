#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>

#ifndef ASSET_DIR
#define ASSET_DIR "assets"
#endif

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out vec2 vTexCoord;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    vTexCoord = aTexCoord;
}
)";

const char* fragmentSrc = R"(
#version 330 core
in vec2 vTexCoord;
uniform sampler2D ourTexture;
out vec4 FragColor;

void main() {
    FragColor = texture(ourTexture, vTexCoord);
}
)";

const glm::vec3 initialCamPos(0.0f, 0.0f, 3.0f);
const glm::vec3 initialCamFront(0.0f, 0.0f, -1.0f);
const glm::vec3 camUp(0.0f, 1.0f, 0.0f);
constexpr float initialYaw = -90.0f;
constexpr float initialPitch = 0.0f;
constexpr float initialFov = 45.0f;

glm::vec3 camPos = initialCamPos;
glm::vec3 camFront = initialCamFront;
float yaw = initialYaw;
float pitch = initialPitch;
float fov = initialFov;
double lastX = 0.0;
double lastY = 0.0;
bool firstMouse = true;
bool invertY = false;

void updateWindowTitle(GLFWwindow* window) {
    char title[128];
    std::snprintf(title, sizeof(title),
                  "Week 11 | yaw=%.1f  pitch=%.1f  fov=%.1f | Invert Y: %s",
                  yaw, pitch, fov, invertY ? "on" : "off");
    glfwSetWindowTitle(window, title);
}

void mouseCallback(GLFWwindow* window, double x, double y) {
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
        return;
    }

    const float xoffset = static_cast<float>(x - lastX);
    float yoffset = static_cast<float>(lastY - y);
    lastX = x;
    lastY = y;
    if (invertY) yoffset = -yoffset;

    constexpr float sensitivity = 0.1f;
    yaw = std::fmod(yaw + xoffset * sensitivity, 360.0f);
    pitch = std::clamp(pitch + yoffset * sensitivity, -89.0f, 89.0f);

    const float yawRadians = glm::radians(yaw);
    const float pitchRadians = glm::radians(pitch);
    const float horizontal = std::cos(pitchRadians);
    camFront = glm::normalize(glm::vec3(std::cos(yawRadians) * horizontal,
                                       std::sin(pitchRadians),
                                       std::sin(yawRadians) * horizontal));
    updateWindowTitle(window);
}

void scrollCallback(GLFWwindow* window, double, double yoffset) {
    fov = std::clamp(fov - static_cast<float>(yoffset), 1.0f, 90.0f);
    updateWindowTitle(window);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    // A held key produces GLFW_REPEAT events. These actions run once per press.
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_I) {
        invertY = !invertY;
        updateWindowTitle(window);
    }
    if (key == GLFW_KEY_P) {
        std::cout << std::fixed << std::setprecision(2)
                  << "camPos   = (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")\n"
                  << "camFront = (" << camFront.x << ", " << camFront.y << ", " << camFront.z << ")\n"
                  << std::setprecision(1)
                  << "yaw = " << yaw << "   pitch = " << pitch << "   fov = " << fov << '\n'
                  << std::setprecision(6)
                  << "length = " << glm::length(camFront) << std::endl;
    }
    if (key == GLFW_KEY_R) {
        camPos = initialCamPos;
        camFront = initialCamFront;
        yaw = initialYaw;
        pitch = initialPitch;
        fov = initialFov;
        updateWindowTitle(window);
    }
}

void processInput(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed = 2.5f * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        speed *= 2.0f;

    const glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += speed * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= speed * camFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= speed * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += speed * right;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camPos += speed * camUp;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) camPos -= speed * camUp;
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

unsigned int loadTexture(const char* path) {
    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Texture load failed: " << path << "\n"
                  << stbi_failure_reason() << std::endl;
        return 0;
    }
    const GLenum formats[] = {0, GL_RED, GL_RG, GL_RGB, GL_RGBA};
    const GLenum format = formats[channels];
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // RGB rows need not be a multiple of four bytes when the image is replaced.
    int previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                 format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
    stbi_image_free(data);
    std::cout << path << "  " << width << "x" << height
              << "  channels: " << channels << std::endl;
    return id;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed.\n";
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(900, 700, "Week 11: Textures", nullptr, nullptr);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetWindowFocusCallback(window, [](GLFWwindow*, int) { firstMouse = true; });
    updateWindowTitle(window);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    const unsigned int shader = createProgram(vertexSrc, fragmentSrc);
    if (!shader) {
        glfwTerminate();
        return 1;
    }
    const int locModel = glGetUniformLocation(shader, "uModel");
    const int locView = glGetUniformLocation(shader, "uView");
    const int locProjection = glGetUniformLocation(shader, "uProjection");

    const float vertices[] = {
        // front +Z (position XYZ, UV)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,
        // back -Z (position XYZ, UV)
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,
        // left -X (position XYZ, UV)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,
        // right +X (position XYZ, UV)
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,
        // bottom -Y (position XYZ, UV)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  1.0f,
        // top +Y (position XYZ, UV)
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,
    };
    static_assert(sizeof(vertices) / sizeof(float) == 36 * 5);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0);
    const unsigned int texture = loadTexture(ASSET_DIR "/container.png");
    if (!texture) {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteProgram(shader);
        glfwTerminate();
        return 1;
    }
    glClearColor(0.08f, 0.09f, 0.13f, 1.0f);

    std::cout << "Mouse: look | Wheel: zoom | WASD: move | Space/C: up/down\n"
              << "Shift or Left Ctrl: 2x speed | I: invert Y | P: print camera | R: reset | Esc: exit"
              << std::endl;
    double lastTime = glfwGetTime();
    float angle = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - lastTime);
        lastTime = now;
        processInput(window, dt);
        angle += dt;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0) continue;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const glm::mat4 projection =
            glm::perspective(glm::radians(fov), static_cast<float>(width) / height, 0.1f, 100.0f);
        const glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);

        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        for (int i = 0; i < 5; ++i) {
            const glm::mat4 model = glm::rotate(
                glm::translate(glm::mat4(1.0f), cubePositions[i]),
                angle * (0.5f + i * 0.3f), glm::vec3(0.3f, 1.0f, 0.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}
