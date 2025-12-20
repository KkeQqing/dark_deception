#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include "Player.h"

// 输入处理类
class InputHandler {
public:
	// 处理输入
    static void ProcessInput(GLFWwindow* window, Player& player, float deltaTime) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float velocity = player.speed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            player.position.y -= velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            player.position.y += velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            player.position.x -= velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            player.position.x += velocity;

    }
};