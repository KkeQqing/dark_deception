#define GLM_ENABLE_EXPERIMENTAL
#include "Player.h"
#include <cmath> // For sqrt

// --- 修改后的构造函数实现 ---
// 接受起始单元格坐标和单元格大小
Player::Player(int startX, int startY, float cellSize) : cellX(startX), cellY(startY) {
    // 初始位置设为中心点
    position = glm::vec2(cellX * cellSize + cellSize / 2.0f, cellY * cellSize + cellSize / 2.0f);
    targetPosition = position; // 初始时，目标就是当前位置
    isMoving = false;
    // speed 和其他初始值保持默认或由外部设置
}


// --- 修改后的 Update 函数 ---
void Player::Update(float deltaTime) {
    // --- 原有技能逻辑保持不变 ---
    if (accelTimer > 0) {
        accelTimer -= deltaTime;
        // speed = 300.0f; // 不再直接改变 speed，而是用 isAccelerating 标志
        isAccelerating = true; // 保持加速标志为真
    }
    else {
        // speed = 150.0f; // 恢复默认速度
        isAccelerating = false; // 关闭加速标志
    }

    if (cooldownE > 0) cooldownE -= deltaTime;
    if (cooldownQ > 0) cooldownQ -= deltaTime;

    // --- 新增：执行移动动画 ---
    if (isMoving) {
        PerformMovement(deltaTime, /* cellSize will be passed from main */ 25.0f); // Placeholder cellSize
        // 注意：cellSize 应该从外部传入，这里暂时用 25.0f 占位，实际应在 main 中调用时传递
        // 更好的方式是在 main 中先调用 PerformMovement 再调用这个 Update
        // 或者将 PerformMovement 的调用移到 main 中
        // 为了清晰，我们假设 PerformMovement 在 main 中被调用
    }
}

// --- 保留原有的 IsInCell 和 CheckWallCollision 函数 ---
// (除非你确定完全不需要它们，否则保留)
bool Player::IsInCell(int cx, int cy, float cellSize) const {
    float left = cx * cellSize;
    float right = (cx + 1) * cellSize;
    float top = cy * cellSize;
    float bottom = (cy + 1) * cellSize;
    return (position.x + radius > left && position.x - radius < right &&
        position.y + radius > top && position.y - radius < bottom);
}

bool Player::CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const {
    int gridX = static_cast<int>(newX / cellSize);
    int gridY = static_cast<int>(newY / cellSize);

    if (gridX < 0 || gridX >= mazeGen.width || gridY < 0 || gridY >= mazeGen.height) {
        return true;
    }

    const Cell& currentCell = mazeGen.maze[gridY][gridX];

    float cellLeft = gridX * cellSize;
    float cellRight = (gridX + 1) * cellSize;
    float cellTop = gridY * cellSize;
    float cellBottom = (gridY + 1) * cellSize;

    float playerLeft = newX - radius;
    float playerRight = newX + radius;
    float playerTop = newY - radius;
    float playerBottom = newY + radius;

    if (currentCell.walls[3] && playerLeft < cellLeft) return true;
    if (currentCell.walls[1] && playerRight > cellRight) return true;
    if (currentCell.walls[0] && playerTop < cellTop) return true;
    if (currentCell.walls[2] && playerBottom > cellBottom) return true;

    return false;
}

// --- 新增方法实现 ---

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

// 尝试触发一次移动
void Player::TryMove(int dx, int dy, const MazeGenerator& mazeGen, float cellSize) {
    // 如果已经在移动，则忽略新的移动指令
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
        // 播放脚步声等音效可以在这里添加
    }
}

// 执行移动动画
void Player::PerformMovement(float deltaTime, float cellSize) {
    if (!isMoving) return;

    // 计算当前速度 (可以根据加速状态调整)
    float currentMoveSpeed = isAccelerating ? moveSpeed * 2.0f : moveSpeed; // 示例：加速时速度翻倍

    // 计算从当前位置到目标位置的方向向量
    glm::vec2 direction = targetPosition - position;
    float distanceToTarget = glm::length(direction);

    // 如果已经非常接近目标位置，则完成移动
    if (distanceToTarget < 1.0f) { // 使用小阈值避免浮点误差
        position = targetPosition;
        isMoving = false;
        // 确保逻辑位置与视觉位置一致
        // cellX/Y 已经在 TryMove 中更新过了
    }
    else {
        // 标准化方向向量
        direction = glm::normalize(direction);
        // 计算本帧应该移动的距离
        float moveDistance = currentMoveSpeed * deltaTime;
        // 更新位置
        position += direction * moveDistance;
        // 如果移动后超过了目标点，则修正到目标点 (可选，增加稳定性)
        if (glm::dot(position - targetPosition, direction) > 0) {
            position = targetPosition;
            isMoving = false;
        }
    }
}