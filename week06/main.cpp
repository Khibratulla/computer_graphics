#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

// ---------- Шейдерлер: бір ғана uniform — mat4 uTransform ----------
const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 uTransform;
out vec3 vColor;
void main() {
    gl_Position = uTransform * vec4(aPos, 1.0);
    vColor = aColor;
}
)";

const char* fragmentSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

// ---------- Режимдер (1-4 пернелері) ----------
// 1 — базалық бөлім:   T · R · S  (орбита + өз центрінен айналу)
// 2 — 1-тапсырма:      R · T · S  (реті ауыстырылған)
// 3 — 2-тапсырма:      сол төменгі бұрыштан айналу
// 4 — 3-тапсырма:      үш объект, бір геометрия
int mode = 1;

// ---------- Жаһандық күй ----------
float speed = 1.5f;      // радиан/секунд
float angle = 0.0f;
float scaleK = 1.0f;     // Z / X — масштабты өзгерту (қосымша)
bool  pulse = false;     // P — пульсация (қосымша)
bool  mirror = false;    // M — айна: scale(-1, 1, 1) (қосымша)
bool  printM = false;    // I — матрицаны консольге шығару (қосымша)

void keyCallback(GLFWwindow*, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4) mode = key - GLFW_KEY_0;
    if (key == GLFW_KEY_P) pulse = !pulse;
    if (key == GLFW_KEY_M) mirror = !mirror;
    if (key == GLFW_KEY_I) printM = !printM;
}

void processInput(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Екеуі де dt-ге тәуелді
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) speed += 2.0f * dt;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) speed -= 2.0f * dt;
    speed = std::clamp(speed, -10.0f, 10.0f);

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) scaleK += 1.0f * dt;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) scaleK -= 1.0f * dt;
    scaleK = std::clamp(scaleK, 0.2f, 2.0f);
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
    return shader;
}

unsigned int createProgram(const char* vsSrc, const char* fsSrc) {
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);

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
    return program;
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Week 06: GLM transforms", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);

    // ---------- Төртбұрыш: 4 вершина, 6 индекс ----------
    float vertices[] = {
        // позиция          // түс
         0.3f,  0.3f, 0.0f,  1.0f, 0.0f, 0.0f,   // 0 — оң жоғарғы
         0.3f, -0.3f, 0.0f,  0.0f, 1.0f, 0.0f,   // 1 — оң төменгі
        -0.3f, -0.3f, 0.0f,  0.0f, 0.0f, 1.0f,   // 2 — сол төменгі
        -0.3f,  0.3f, 0.0f,  1.0f, 1.0f, 0.0f    // 3 — сол жоғарғы
    };
    unsigned int indices[] = { 0, 1, 3,  1, 2, 3 };

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    const int STRIDE = 6 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    unsigned int shader = createProgram(vertexSrc, fragmentSrc);

    // Uniform орнын циклге дейін бір рет табамыз
    int locTransform = glGetUniformLocation(shader, "uTransform");

    const glm::vec3 zAxis(0.0f, 0.0f, 1.0f);
    double lastTime = glfwGetTime();
    double timeAcc = 0.0;
    int lastMode = 0;

    std::cout << "1: T*R*S  2: R*T*S  3: бұрыштан айналу  4: үш объект\n"
              << "Q/E: жылдамдық  Z/X: масштаб  P: пульсация  M: айна  I: матрица\n";

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = (float)(now - lastTime);
        lastTime = now;
        timeAcc += dt;

        processInput(window, dt);
        angle += speed * dt;

        if (mode != lastMode) {
            lastMode = mode;
            std::cout << "Режим: " << mode << std::endl;
        }

        float sc = (pulse ? 0.5f + 0.15f * std::sin((float)timeAcc * 3.0f) : 0.6f) * scaleK;
        float sx = mirror ? -sc : sc;

        glClearColor(0.08f, 0.09f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);
        glBindVertexArray(vao);

        if (mode == 4) {
            // 3-тапсырма: геометрия бір, матрица үш рет құрылады
            for (int i = 0; i < 3; i++) {
                glm::mat4 m = glm::mat4(1.0f);   // цикл ІШІНДЕ — әр кадр, әр объект жаңадан
                m = glm::translate(m, glm::vec3(-0.5f + i * 0.5f, 0.0f, 0.0f));
                m = glm::rotate(m, angle * (i + 1), zAxis);
                m = glm::scale(m, glm::vec3(0.4f * scaleK * (mirror ? -1.0f : 1.0f), 0.4f * scaleK, 1.0f));
                glUniformMatrix4fv(locTransform, 1, GL_FALSE, glm::value_ptr(m));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        } else {
            glm::mat4 m = glm::mat4(1.0f);       // цикл ІШІНДЕ
            glm::vec3 pos(std::cos(angle) * 0.4f, std::sin(angle) * 0.4f, 0.0f);

            if (mode == 1) {
                // T · R · S: алдымен орбитаға апарылады, бірақ айналу өз центрінен
                m = glm::translate(m, pos);
                m = glm::rotate(m, angle, zAxis);
                m = glm::scale(m, glm::vec3(sx, sc, 1.0f));
            } else if (mode == 2) {
                // R · T · S: айналу координат басы төңірегінде — орбита қисаяды
                m = glm::rotate(m, angle, zAxis);
                m = glm::translate(m, pos);
                m = glm::scale(m, glm::vec3(sx, sc, 1.0f));
            } else {
                // Сол төменгі бұрыш (-0.3, -0.3) қозғалмайды, қалғаны соны айнала бұрылады
                m = glm::translate(m, glm::vec3(-0.3f, -0.3f, 0.0f));   // 3-ші әсер етеді
                m = glm::rotate(m, angle, zAxis);                       // 2-ші
                m = glm::translate(m, glm::vec3( 0.3f,  0.3f, 0.0f));   // 1-ші әсер етеді
                m = glm::scale(m, glm::vec3(mirror ? -scaleK : scaleK, scaleK, 1.0f));
            }

            if (printM)
                std::cout << "m[3][0]=" << m[3][0] << "  m[3][1]=" << m[3][1] << std::endl;

            glUniformMatrix4fv(locTransform, 1, GL_FALSE, glm::value_ptr(m));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shader);
    glfwTerminate();
}
