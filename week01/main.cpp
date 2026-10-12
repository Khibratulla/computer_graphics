#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        float t = (float)glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        else
            glClearColor((sin(t) + 1) * 0.15f, (sin(t * 1.5f) + 1) * 0.15f, 0.35f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT);

        frames++;
        if (glfwGetTime() - lastTime >= 1.0) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            lastTime = glfwGetTime();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
