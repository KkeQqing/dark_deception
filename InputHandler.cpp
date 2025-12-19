// src/InputHandler.cpp
#include "InputHandler.h"

std::array<bool, 512> InputHandler::keys = {};
double InputHandler::mouseX = 0.0;
double InputHandler::mouseY = 0.0;

void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 512) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }
}

void InputHandler::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
}

void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // 可以在这里记录鼠标按键状态，或者直接在回调里处理
}

bool InputHandler::isKeyPressed(int key) {
    if (key >= 0 && key < 512) {
        return keys[key];
    }
    return false;
}

bool InputHandler::isMouseButtonPressed(int button) {
    // 示例：检查左键
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        return glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_PRESS;
    }
    return false;
}

void InputHandler::update() {
    // 当前简单实现不需要每帧更新
}