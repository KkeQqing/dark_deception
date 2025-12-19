#pragma once
#include <glad/glad.h> // 必须在 glfw3.h 之前
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "Shader.h"
#include "MazeGenerator.h"
#include "Player.h"
#include "Monster.h"
#include "Collectible.h"
#define M_PI 3.14159265358979323846

// 渲染器类
class Renderer {
public:
	Shader shader;// 着色器程序
	unsigned int VAO, VBO; // 顶点数组对象和顶点缓冲对象
	glm::mat4 projection; // 投影矩阵

	// 构造函数，初始化渲染器
    Renderer(int screenWidth, int screenHeight) : shader("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl") {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        projection = glm::ortho(0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, -1.0f, 1.0f);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); // Initial empty buffer

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

	// 析构函数，释放资源
    ~Renderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

	// 开始渲染帧
    void BeginFrame() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        shader.use();
        shader.setMat4("projection", projection);
    }

	// 结束渲染帧
    void EndFrame(GLFWwindow* window) {
        glfwSwapBuffers(window);
    }

	// 绘制迷宫
    void DrawMaze(const MazeGenerator& mazeGen, float cellSize) {
        glBindVertexArray(VAO);
        std::vector<float> vertices;
        for (int y = 0; y < mazeGen.height; ++y) {
            for (int x = 0; x < mazeGen.width; ++x) {
                const Cell& cell = mazeGen.maze[y][x];
                float x1 = x * cellSize;
                float x2 = (x + 1) * cellSize;
                float y1 = y * cellSize;
                float y2 = (y + 1) * cellSize;

                if (cell.walls[0]) { // Top
                    vertices.insert(vertices.end(), { x1, y1, 0.8f, 0.8f, 0.8f, x2, y1, 0.8f, 0.8f, 0.8f });
                }
                if (cell.walls[1]) { // Right
                    vertices.insert(vertices.end(), { x2, y1, 0.8f, 0.8f, 0.8f, x2, y2, 0.8f, 0.8f, 0.8f });
                }
                if (cell.walls[2]) { // Bottom
                    vertices.insert(vertices.end(), { x2, y2, 0.8f, 0.8f, 0.8f, x1, y2, 0.8f, 0.8f, 0.8f });
                }
                if (cell.walls[3]) { // Left
                    vertices.insert(vertices.end(), { x1, y2, 0.8f, 0.8f, 0.8f, x1, y1, 0.8f, 0.8f, 0.8f });
                }
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, vertices.size() / 5);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

	// 绘制玩家
    void DrawPlayer(const Player& player) {
        DrawCircle(player.position.x, player.position.y, player.radius, 1.0f, 0.0f, 0.0f);
    }

	// 绘制怪物
    void DrawMonsters(const std::vector<Monster>& monsters) {
        for (const auto& monster : monsters) {
            if (monster.visible) {
                DrawTriangle(monster.position.x, monster.position.y, monster.radius, 0.0f, 0.0f, 1.0f);
            }
        }
    }

	// 绘制收集物
    void DrawCollectibles(const std::vector<Collectible>& collectibles) {
        for (const auto& item : collectibles) {
            if (!item.collected) {
                DrawSquare(item.position.x, item.position.y, item.size, 1.0f, 0.0f, 1.0f);
            }
        }
    }

	// 绘制警报闪烁效果
    void DrawAlertFlash() {
        // 简单的全屏红色清屏作为闪烁效果
        glClearColor(0.8f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // 重置背景色
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }


private:
	// 绘制圆形
    void DrawCircle(float cx, float cy, float r, float red, float green, float blue) {
        glBindVertexArray(VAO);
        std::vector<float> vertices;
        const int segments = 32;
        vertices.insert(vertices.end(), { cx, cy, red, green, blue }); // Center point
        for (int i = 0; i <= segments; ++i) {
            float angle = i * 2.0f * static_cast<float>(M_PI) / segments;
            float x = cx + r * cos(angle);
            float y = cy + r * sin(angle);
            vertices.insert(vertices.end(), { x, y, red, green, blue });
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 5);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

	// 绘制方形
    void DrawSquare(float cx, float cy, float size, float red, float green, float blue) {
        glBindVertexArray(VAO);
        float half = size / 2.0f;
        float vertices[] = {
            cx - half, cy - half, red, green, blue,
            cx + half, cy - half, red, green, blue,
            cx + half, cy + half, red, green, blue,
            cx - half, cy + half, red, green, blue
        };
        unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

        unsigned int EBO;
        glGenBuffers(1, &EBO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glDeleteBuffers(1, &EBO);
    }

	// 绘制三角形
    void DrawTriangle(float cx, float cy, float r, float red, float green, float blue) {
        glBindVertexArray(VAO);
        float vertices[] = {
            cx, cy + r * 0.8f, red, green, blue, // Top
            cx - r * 0.7f, cy - r * 0.4f, red, green, blue, // Bottom Left
            cx + r * 0.7f, cy - r * 0.4f, red, green, blue  // Bottom Right
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};