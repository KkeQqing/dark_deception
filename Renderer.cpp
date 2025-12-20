#include "Renderer.h"

// --- 修改: 构造函数实现 ---
// Renderer::Renderer(int screenWidth, int screenHeight) : shader("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl") {
Renderer::Renderer(int screenWidth, int screenHeight, float worldWidth, float worldHeight) : shader("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl") {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // --- 修改: 使用世界尺寸设置正交投影矩阵 ---
    // projection = glm::ortho(0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, -1.0f, 1.0f); // 旧
    projection = glm::ortho(0.0f, worldWidth, worldHeight, 0.0f, -1.0f, 1.0f); // 新: 基于迷宫世界的尺寸

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

// 析构函数实现
Renderer::~Renderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// 开始渲染帧
void Renderer::BeginFrame() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    shader.use();
    shader.setMat4("projection", projection);
}

// 结束渲染帧
void Renderer::EndFrame(GLFWwindow* window) {
    glfwSwapBuffers(window);
}

// 绘制迷宫
void Renderer::DrawMaze(const MazeGenerator& mazeGen, float cellSize) {
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
void Renderer::DrawPlayer(const Player& player) {
    DrawCircle(player.position.x, player.position.y, player.radius, 1.0f, 0.0f, 0.0f);
}

// 绘制怪物
void Renderer::DrawMonsters(const std::vector<Monster>& monsters) {
    for (const auto& monster : monsters) {
        if (monster.visible) {
            DrawTriangle(monster.position.x, monster.position.y, monster.radius, 0.0f, 0.0f, 1.0f);
        }
    }
}

// 绘制收集物
void Renderer::DrawCollectibles(const std::vector<Collectible>& collectibles) {
    for (const auto& item : collectibles) {
        if (!item.collected) {
            DrawSquare(item.position.x, item.position.y, item.size, 1.0f, 0.0f, 1.0f);
        }
    }
}

// 绘制警报闪烁效果
void Renderer::DrawAlertFlash() {
    // 简单的全屏红色清屏作为闪烁效果
    glClearColor(0.8f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // 重置背景色 (虽然下一帧BeginFrame会再次设置，但保持一致性好)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}


// --- 私有函数实现 ---

// 绘制圆形
void Renderer::DrawCircle(float cx, float cy, float r, float red, float green, float blue) {
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
void Renderer::DrawSquare(float cx, float cy, float size, float red, float green, float blue) {
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
void Renderer::DrawTriangle(float cx, float cy, float r, float red, float green, float blue) {
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