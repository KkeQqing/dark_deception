#define GLM_ENABLE_EXPERIMENTAL
#include "Player.h"
#include <cmath> 

Player::Player(int startX, int startY, float cellSize) : cellX(startX), cellY(startY) {
    // 初始位置设为中心点
    position = glm::vec2(cellX * cellSize + cellSize / 2.0f, cellY * cellSize + cellSize / 2.0f);
    targetPosition = position;
    isMoving = false;
}


// --- Update 函数 ---
void Player::Update(float deltaTime) {
    if (accelTimer > 0) {
        accelTimer -= deltaTime;
        isAccelerating = true; 
    }
    else {
        isAccelerating = false; 
    }

    if (cooldownE > 0) cooldownE -= deltaTime;
    if (cooldownQ > 0) cooldownQ -= deltaTime;

    // --- 执行移动动画 ---
    if (isMoving) {
        PerformMovement(deltaTime,25.0f); 
    }
}

bool Player::IsInCell(int cx, int cy, float cellSize) const {
    float left = cx * cellSize;
    float right = (cx + 1) * cellSize;
    float top = cy * cellSize;
    float bottom = (cy + 1) * cellSize;
    return (position.x + radius > left && position.x - radius < right &&
        position.y + radius > top && position.y - radius < bottom);
}

// 检查是否可以从当前位置移动到指定单元格
bool Player::CanMoveTo(int newCellX, int newCellY, const MazeGenerator& mazeGen) const {
    // 边界检查
    if (newCellX < 0 || newCellX >= mazeGen.width || newCellY < 0 || newCellY >= mazeGen.height) {
        return false;
    }

    // 获取当前单元格
    const Cell& currentCell = mazeGen.maze[cellY][cellX];
    // 获取目标单元格
    const Cell& targetCell = mazeGen.maze[newCellY][newCellX];

    // 计算移动方向
    int dx = newCellX - cellX;
    int dy = newCellY - cellY;

    // 检查墙壁 (确保两个相邻单元格之间的墙都被打通)
    if (dx == 1) { // 向右移动
        return !currentCell.walls[1] && !targetCell.walls[3];
    }
    else if (dx == -1) { // 向左移动
        return !currentCell.walls[3] && !targetCell.walls[1];
    }
    else if (dy == 1) { // 向下移动
        return !currentCell.walls[2] && !targetCell.walls[0];
    }
    else if (dy == -1) { // 向上移动
        return !currentCell.walls[0] && !targetCell.walls[2];
    }

    // 不是相邻单元格或者没有有效方向
    return false;
}

void Player::TryMove(int dx, int dy, const MazeGenerator& mazeGen, float cellSize) {
    if (isMoving) return;

    int newCellX = cellX + dx;
    int newCellY = cellY + dy;

    // 检查是否可以移动到目标单元格
    if (CanMoveTo(newCellX, newCellY, mazeGen)) {
        // 设置移动状态
        isMoving = true;
        cellX = newCellX; // 更新逻辑上的单元格坐标
        cellY = newCellY;
        // 设置目标像素位置为新单元格的中心
        targetPosition = glm::vec2(cellX * cellSize + cellSize / 2.0f, cellY * cellSize + cellSize / 2.0f);
    }
}

// 执行移动动画
void Player::PerformMovement(float deltaTime, float cellSize) {
    if (!isMoving) return;

    // 计算当前速度
	float currentMoveSpeed = isAccelerating ? moveSpeed * 2.0f : moveSpeed; // 加速时速度翻倍

    // 计算从当前位置到目标位置的方向向量
    glm::vec2 direction = targetPosition - position;
    float distanceToTarget = glm::length(direction);

    if (distanceToTarget < 1.0f) {
        position = targetPosition;
        isMoving = false;
    }
    else {
        direction = glm::normalize(direction);
        // 计算本帧应该移动的距离
        float moveDistance = currentMoveSpeed * deltaTime;
        // 更新位置
        position += direction * moveDistance;
        if (glm::dot(position - targetPosition, direction) > 0) {
            position = targetPosition;
            isMoving = false;
        }
    }
}