// src/InputHandler.h
#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <GLFW/glfw3.h>
#include <array>

class InputHandler {
public:
    static std::array<bool, 512> keys;
    static double mouseX, mouseY;

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    static bool isKeyPressed(int key);
    static bool isMouseButtonPressed(int button);
    static void update(); // 可用于每帧重置某些状态
};

#endif