#define GLM_ENABLE_EXPERIMENTAL
#include "Player.h"
#include <cmath> // For sqrt

// 构造函数实现
Player::Player(float x, float y) : position(x, y) {}

// 更新玩家状态
void Player::Update(float deltaTime) {
    if (accelTimer > 0) {
        accelTimer -= deltaTime;
        speed = 300.0f;
    }
    else {
        speed = 150.0f;
        isAccelerating = false;
    }

    if (cooldownE > 0) cooldownE -= deltaTime;
    if (cooldownQ > 0) cooldownQ -= deltaTime;
}

// 检测玩家是否在指定单元格内
bool Player::IsInCell(int cx, int cy, float cellSize) const {
    float left = cx * cellSize;
    float right = (cx + 1) * cellSize;
    float top = cy * cellSize;
    float bottom = (cy + 1) * cellSize;
    return (position.x + radius > left && position.x - radius < right &&
        position.y + radius > top && position.y - radius < bottom);
}

// --- 新增: 精确墙体碰撞检测 ---
bool Player::CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const {
    // 计算玩家中心所在的网格单元格
    int gridX = static_cast<int>(newX / cellSize);
    int gridY = static_cast<int>(newY / cellSize);

    // 边界检查 (假设迷宫外是墙)
    if (gridX < 0 || gridX >= mazeGen.width || gridY < 0 || gridY >= mazeGen.height) {
        return true; // 撞墙
    }

    const Cell& currentCell = mazeGen.maze[gridY][gridX];

    // 检查与单元格四壁的距离
    float cellLeft = gridX * cellSize;
    float cellRight = (gridX + 1) * cellSize;
    float cellTop = gridY * cellSize;
    float cellBottom = (gridY + 1) * cellSize;

    // 玩家边界
    float playerLeft = newX - radius;
    float playerRight = newX + radius;
    float playerTop = newY - radius;
    float playerBottom = newY + radius;

    // 如果玩家的边界超出了单元格的边界，并且该侧有墙，则碰撞
    if (currentCell.walls[3] && playerLeft < cellLeft) return true; // 左墙
    if (currentCell.walls[1] && playerRight > cellRight) return true; // 右墙
    if (currentCell.walls[0] && playerTop < cellTop) return true; // 上墙
    if (currentCell.walls[2] && playerBottom > cellBottom) return true; // 下墙

    return false; // 没有碰撞
}