#include <iostream>
#include <GLFW/glfw3.h>
#include <GL/glew.h>

// 窗口大小
const int WIDTH = 800;
const int HEIGHT = 600;

// 地球的旋转角度
float earthRotation = 0.0f;

// 初始化OpenGL
void initOpenGL() {
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(-1);
    }
}

// 渲染函数
void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 设置模型视图矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(earthRotation, 0.0f, 1.0f, 0.0f); // 绕Y轴旋转

    // 绘制地球（这里假设你有一个绘制地球的函数）
    drawEarth();

    glfwSwapBuffers(glfwGetCurrentContext());
}

// 处理窗口大小变化
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 主循环
void mainLoop() {
    while (!glfwWindowShouldClose(glfwGetCurrentContext())) {
        render();

        glfwPollEvents();
    }
}

int main() {
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "3D Earth", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 设置窗口大小变化回调
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化OpenGL
    initOpenGL();

    // 主循环
    mainLoop();

    // 清理
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}