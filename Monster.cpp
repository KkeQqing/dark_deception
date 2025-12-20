#include "Monster.h"
#include "Player.h" // 包含 Player 定义
#include <cmath>   // 用于 sqrt, abs, floor
#include <algorithm> // 用于 std::reverse
#include <climits> // 用于 INT_MAX
#include <cstring> // 用于 memset (可选, 但对于重置数组很有用)
#include <random>  // 用于随机数生成
#include <glm/gtc/constants.hpp> // 可能需要这个头文件用于 PI

// --- 辅助函数: Monster 移动的简单墙体碰撞检测 ---
bool Monster::CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const {
    // 计算怪物中心所在的网格单元格
    int gridX = static_cast<int>(floor(newX / cellSize));
    int gridY = static_cast<int>(floor(newY / cellSize));

    // 边界检查 (假设迷宫外是墙壁)
    if (gridX < 0 || gridX >= mazeGen.width || gridY < 0 || gridY >= mazeGen.height) {
        return true; // 撞墙
    }

    const Cell& currentCell = mazeGen.maze[gridY][gridX];

    // 检查相对于单元格边界的位置
    float cellLeft = gridX * cellSize;
    float cellRight = (gridX + 1) * cellSize;
    float cellTop = gridY * cellSize;
    float cellBottom = (gridY + 1) * cellSize;

    // 怪物边界
    float monsterLeft = newX - radius;
    float monsterRight = newX + radius;
    float monsterTop = newY - radius;
    float monsterBottom = newY + radius;

    // 如果怪物边界超出单元格边界并且该边存在墙壁，则发生碰撞
    if (currentCell.walls[3] && monsterLeft < cellLeft + 1.0f) return true; // 左墙 (添加小缓冲区)
    if (currentCell.walls[1] && monsterRight > cellRight - 1.0f) return true; // 右墙
    if (currentCell.walls[0] && monsterTop < cellTop + 1.0f) return true; // 上墙
    if (currentCell.walls[2] && monsterBottom > cellBottom - 1.0f) return true; // 下墙

    return false; // 无碰撞
}

// --- 辅助函数: 检查是否可以向给定方向移动一步 ---
bool Monster::CanMoveInDirection(const MazeGenerator& mazeGen, Direction dir, float cellSize) const {
    float testX = position.x;
    float testY = position.y;
    // 估计一小步的距离 (假设约 60 FPS)
    float stepDistance = currentSpeed * (1.0f / 60.0f);

    switch (dir) {
    case UP:    testY -= stepDistance; break; // 上
    case RIGHT: testX += stepDistance; break; // 右
    case DOWN:  testY += stepDistance; break; // 下
    case LEFT:  testX -= stepDistance; break; // 左
    default: return false; // NONE 或无效方向
    }

    // 检查在新的潜在位置是否有碰撞
    return !CheckWallCollision(mazeGen, testX, testY, cellSize);
}

// --- 辅助函数: 获取巡逻用的随机有效方向 ---
Monster::Direction Monster::GetRandomValidDirection(const MazeGenerator& mazeGen, float cellSize) const {
    std::vector<Direction> validDirections;
    // 检查所有四个方向
    for (int d = UP; d <= LEFT; ++d) {
        Direction dir = static_cast<Direction>(d);
        if (CanMoveInDirection(mazeGen, dir, cellSize)) {
            validDirections.push_back(dir);
        }
    }

    if (!validDirections.empty()) {
        // 选择一个随机的有效方向
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, validDirections.size() - 1);
        return validDirections[dis(gen)];
    }
    return NONE; // 未找到有效方向
}

// --- 构造函数实现 ---
Monster::Monster(float x, float y)
    : position(x, y),
    homePosition(x, y) // 设置巡逻中心点
{
    // 初始化路径索引
    currentPathIndex = 0;
    // 随机初始化方向
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(minDirectionChangeInterval, maxDirectionChangeInterval);
    // 以一个随机计时器开始
    directionChangeTimer = dis(gen);
}

// --- 更新怪物状态 ---
void Monster::Update(float deltaTime, const Player& player, const MazeGenerator& mazeGen, float cellSize) {
    if (frozen) {
        // 冻结状态: 不移动
        return;
    }

    float dx_to_player = player.position.x - position.x;
    float dy_to_player = player.position.y - position.y;
    // 计算到玩家的距离
    float distanceToPlayer = sqrt(dx_to_player * dx_to_player + dy_to_player * dy_to_player);

    // --- 修改: 结合视线检测确定状态 ---
    // 检查玩家是否在探测范围内并且视线无障碍
    bool playerInRange = distanceToPlayer < detectionRange;
    bool playerVisible = false;
    if (playerInRange) {
        // 检查视线是否被阻挡
        playerVisible = HasLineOfSight(mazeGen, position.x, position.y, player.position.x, player.position.y, cellSize);
    }

    if (playerInRange && playerVisible) {
        // 如果玩家在范围内且可见，则追逐
        state = MonsterState::CHASING;
        currentSpeed = chaseSpeed; // 使用追逐速度
        visible = true; // 可见
    }
    else {
        // 否则继续巡逻
        state = MonsterState::PATROLLING;
        currentSpeed = baseSpeed; // 使用基础速度
        // 可选项: 即使技术上在范围内但看不到时，让远处的怪物不太显眼
        visible = (distanceToPlayer < 150.0f); // 在一定距离内才可见
    }


    // --- 移动逻辑 ---
    float stepDistance = currentSpeed * deltaTime; // 根据时间和速度计算步长
    float newX = position.x;
    float newY = position.y;

    if (state == MonsterState::CHASING) {
        // --- 追逐模式: 向玩家移动 ---
        // 优先沿距离差较大的轴移动
        if (abs(dx_to_player) > abs(dy_to_player)) {
            // 优先尝试水平移动
            if (dx_to_player > 0) {
                newX += stepDistance; // 向右移动
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newX = position.x; // 如果碰撞则还原
                    // 然后尝试垂直移动
                    if (dy_to_player > 0) newY += stepDistance; // 向下
                    else if (dy_to_player < 0) newY -= stepDistance; // 向上
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newY = position.y; // 如果碰撞则还原
                    }
                }
            }
            else if (dx_to_player < 0) {
                newX -= stepDistance; // 向左移动
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newX = position.x; // 如果碰撞则还原
                    // 然后尝试垂直移动
                    if (dy_to_player > 0) newY += stepDistance; // 向下
                    else if (dy_to_player < 0) newY -= stepDistance; // 向上
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newY = position.y; // 如果碰撞则还原
                    }
                }
            }
        }
        else {
            // 优先尝试垂直移动
            if (dy_to_player > 0) {
                newY += stepDistance; // 向下移动
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newY = position.y; // 如果碰撞则还原
                    // 然后尝试水平移动
                    if (dx_to_player > 0) newX += stepDistance; // 向右
                    else if (dx_to_player < 0) newX -= stepDistance; // 向左
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newX = position.x; // 如果碰撞则还原
                    }
                }
            }
            else if (dy_to_player < 0) {
                newY -= stepDistance; // 向上移动
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newY = position.y; // 如果碰撞则还原
                    // 然后尝试水平移动
                    if (dx_to_player > 0) newX += stepDistance; // 向右
                    else if (dx_to_player < 0) newX -= stepDistance; // 向左
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newX = position.x; // 如果碰撞则还原
                    }
                }
            }
        }
        // 最终检查，以防组合移动失败
        if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
            newX = position.x;
            newY = position.y;
        }

    }
    else { // PATROLLING (巡逻)
        // --- 巡逻模式: 沿着偏好方向移动 ---
        directionChangeTimer -= deltaTime; // 更新方向改变计时器

        // 如果计时器到期、没有当前方向或当前方向被阻挡，则改变方向
        if (directionChangeTimer <= 0.0f || currentDirection == NONE || !CanMoveInDirection(mazeGen, currentDirection, cellSize)) {
            currentDirection = GetRandomValidDirection(mazeGen, cellSize); // 获取新方向
            // 重置计时器，为下次方向改变做准备
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(minDirectionChangeInterval, maxDirectionChangeInterval);
            directionChangeTimer = dis(gen);
        }

        // 根据当前方向执行移动
        if (currentDirection != NONE) {
            switch (currentDirection) {
            case UP:    newY -= stepDistance; break; // 上
            case RIGHT: newX += stepDistance; break; // 右
            case DOWN:  newY += stepDistance; break; // 下
            case LEFT:  newX -= stepDistance; break; // 左
            }

            // 尝试移动后检查碰撞
            if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                // 如果发生碰撞，还原移动
                newX = position.x;
                newY = position.y;
                // 强制在下次更新时改变方向
                directionChangeTimer = 0.0f;
            }
        }
    }

    // 应用最终计算出的位置
    position.x = newX;
    position.y = newY;
}

// --- 未使用的寻路函数的占位符实现 ---
// 如果当前不使用 A* 寻路，这些可以删除或留空。
float Monster::Heuristic(int x1, int y1, int x2, int y2) const {
    // 曼哈顿距离启发式函数
    return static_cast<float>(abs(x1 - x2) + abs(y1 - y2));
}

bool Monster::IsPathBlockedByWall(const MazeGenerator& /*mazeGen*/, float /*fromX*/, float /*fromY*/, float /*toX*/, float /*toY*/, float /*cellSize*/) const {
    // 占位符，此修订版逻辑中未使用
    return false;
}

bool Monster::FindPath(const MazeGenerator& /*mazeGen*/, float /*cellSize*/, const glm::vec2& /*targetPos*/) {
    // 占位符，此修订版逻辑中未使用
    path.clear();
    return false; // 表示未使用/寻路失败
}

// --- 新增: 视线检测 (Line-of-Sight, LOS) 实现 ---
bool Monster::HasLineOfSight(const MazeGenerator& mazeGen, float startX, float startY, float endX, float endY, float cellSize) const {
    // 适应于基于网格碰撞检测的 DDA (Digital Differential Analyzer) 算法

    float dx = endX - startX;
    float dy = endY - startY;
    // 根据网格大小估算步数
    int steps = static_cast<int>(std::ceil(std::max(std::abs(dx), std::abs(dy)) / cellSize));

    if (steps == 0) steps = 1; // 确保至少检查起始单元格

    // 计算每一步在 X 和 Y 方向上的增量
    float xIncrement = dx / steps;
    float yIncrement = dy / steps;

    float currentX = startX;
    float currentY = startY;

    // 遍历光线路径上的步骤
    for (int i = 0; i <= steps; ++i) {
        // 计算当前点所在的网格单元格
        int gridX = static_cast<int>(std::floor(currentX / cellSize));
        int gridY = static_cast<int>(std::floor(currentY / cellSize));

        // 边界检查
        if (gridX < 0 || gridX >= mazeGen.width || gridY < 0 || gridY >= mazeGen.height) {
            // 将迷宫外区域视为阻挡
            return false;
        }

        const Cell& cell = mazeGen.maze[gridY][gridX];

        // 定义单元格边界
        float cellLeft = gridX * cellSize;
        float cellRight = (gridX + 1) * cellSize;
        float cellTop = gridY * cellSize;
        float cellBottom = (gridY + 1) * cellSize;

        // --- 检查光线段在此单元格内是否穿过任何墙壁 ---

        // 1. 检查垂直墙壁穿越 (左/右)
        if (xIncrement > 0) { // 光线向右移动
            // 潜在与下一单元格左墙的交点
            if (gridX + 1 < mazeGen.width) {
                float t = (cellRight - currentX) / xIncrement;
                // 检查交点是否在线段上 (0 <= t <= 1)
                if (t >= 0 && t <= 1) {
                    float intersectY = currentY + t * yIncrement;
                    // 检查交点 Y 坐标是否在墙的垂直跨度内
                    if (intersectY >= cellTop && intersectY <= cellBottom) {
                        // 检查墙壁是否存在
                        if (mazeGen.maze[gridY][gridX].walls[1]) { // 当前单元格的右墙
                            return false; // 被右墙阻挡
                        }
                    }
                }
            }
        }
        else if (xIncrement < 0) { // 光线向左移动
            // 潜在与上一单元格右墙的交点
            if (gridX - 1 >= 0) {
                float t = (cellLeft - currentX) / xIncrement;
                if (t >= 0 && t <= 1) {
                    float intersectY = currentY + t * yIncrement;
                    if (intersectY >= cellTop && intersectY <= cellBottom) {
                        if (mazeGen.maze[gridY][gridX].walls[3]) { // 当前单元格的左墙
                            return false; // 被左墙阻挡
                        }
                    }
                }
            }
        }

        // 2. 检查水平墙壁穿越 (上/下)
        if (yIncrement > 0) { // 光线向下移动
            // 潜在与下一单元格顶墙的交点
            if (gridY + 1 < mazeGen.height) {
                float t = (cellBottom - currentY) / yIncrement;
                if (t >= 0 && t <= 1) {
                    float intersectX = currentX + t * xIncrement;
                    if (intersectX >= cellLeft && intersectX <= cellRight) {
                        if (mazeGen.maze[gridY][gridX].walls[2]) { // 当前单元格的底墙
                            return false; // 被底墙阻挡
                        }
                    }
                }
            }
        }
        else if (yIncrement < 0) { // 光线向上移动
            // 潜在与上一单元格底墙的交点
            if (gridY - 1 >= 0) {
                float t = (cellTop - currentY) / yIncrement;
                if (t >= 0 && t <= 1) {
                    float intersectX = currentX + t * xIncrement;
                    if (intersectX >= cellLeft && intersectX <= cellRight) {
                        if (mazeGen.maze[gridY][gridX].walls[0]) { // 当前单元格的顶墙
                            return false; // 被顶墙阻挡
                        }
                    }
                }
            }
        }

        // 沿着光线移动到下一个采样点
        currentX += xIncrement;
        currentY += yIncrement;
    }

    // 如果在整个遍历过程中都没有撞到墙，则视线畅通
    return true;
}