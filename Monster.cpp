#include "Monster.h"
#include "Player.h"
#include <cmath>
#include <random>

// --- 墙体碰撞检测 ---
bool Monster::CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const {
	int gridX = static_cast<int>(std::floor(newX / cellSize));//坐标转换为网格索引
    int gridY = static_cast<int>(std::floor(newY / cellSize));

    if (gridX < 0 || gridX >= mazeGen.width || gridY < 0 || gridY >= mazeGen.height) {
        return true;
    }

    const Cell& cell = mazeGen.maze[gridY][gridX];
    float cellLeft = gridX * cellSize;
    float cellRight = (gridX + 1) * cellSize;
    float cellTop = gridY * cellSize;
    float cellBottom = (gridY + 1) * cellSize;

    float monsterLeft = newX - radius;
    float monsterRight = newX + radius;
    float monsterTop = newY - radius;
    float monsterBottom = newY + radius;

    if (cell.walls[3] && monsterLeft < cellLeft + 1.0f) return true; // 左
    if (cell.walls[1] && monsterRight > cellRight - 1.0f) return true; // 右
    if (cell.walls[0] && monsterTop < cellTop + 1.0f) return true; // 上
    if (cell.walls[2] && monsterBottom > cellBottom - 1.0f) return true; // 下

    return false;
}

// --- 检查方向是否可行 ---
bool Monster::CanMoveInDirection(const MazeGenerator& mazeGen, Direction dir, float cellSize) const {
    float testX = position.x;
    float testY = position.y;
    float stepDistance = currentSpeed * (1.0f / 60.0f); // 预估一步

    switch (dir) {
    case UP:    testY -= stepDistance; break;
    case RIGHT: testX += stepDistance; break;
    case DOWN:  testY += stepDistance; break;
    case LEFT:  testX -= stepDistance; break;
    default: return false;
    }

    return !CheckWallCollision(mazeGen, testX, testY, cellSize);
}

// --- 获取随机有效巡逻方向 ---
Monster::Direction Monster::GetRandomValidDirection(const MazeGenerator& mazeGen, float cellSize) const {
    std::vector<Direction> validDirs;
    for (int d = UP; d <= LEFT; ++d) {
        Direction dir = static_cast<Direction>(d);
        if (CanMoveInDirection(mazeGen, dir, cellSize)) {
            validDirs.push_back(dir);
        }
    }

    if (!validDirs.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, static_cast<int>(validDirs.size() - 1));
        return validDirs[dis(gen)];
    }
    return NONE;
}

// --- 构造函数 ---
Monster::Monster(float x, float y)
    : position(x, y), homePosition(x, y) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(minDirectionChangeInterval, maxDirectionChangeInterval);
    directionChangeTimer = dis(gen);
}

// --- 更新逻辑 ---
void Monster::Update(float deltaTime, const Player& player, const MazeGenerator& mazeGen, float cellSize) {
    if (frozen) return;

    float dx = player.position.x - position.x;
    float dy = player.position.y - position.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    bool playerInRange = dist < detectionRange;
    bool playerVisible = playerInRange && HasLineOfSight(mazeGen, position.x, position.y, player.position.x, player.position.y, cellSize);

    if (playerInRange && playerVisible) {
        state = MonsterState::CHASING;
        currentSpeed = chaseSpeed;
        visible = true;
    }
    else {
        state = MonsterState::PATROLLING;
        currentSpeed = baseSpeed;
        visible = (dist < 1150.0f);
    }

    float step = currentSpeed * deltaTime;
    float newX = position.x;
    float newY = position.y;

    if (state == MonsterState::CHASING) {
        if (std::abs(dx) > std::abs(dy)) {
            // 优先水平
            if (dx > 0) {
                newX += step;
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newX = position.x;
                    if (dy > 0) newY += step;
                    else if (dy < 0) newY -= step;
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newY = position.y;
                    }
                }
            }
            else if (dx < 0) {
                newX -= step;
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newX = position.x;
                    if (dy > 0) newY += step;
                    else if (dy < 0) newY -= step;
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newY = position.y;
                    }
                }
            }
        }
        else {
            // 优先垂直
            if (dy > 0) {
                newY += step;
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newY = position.y;
                    if (dx > 0) newX += step;
                    else if (dx < 0) newX -= step;
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newX = position.x;
                    }
                }
            }
            else if (dy < 0) {
                newY -= step;
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newY = position.y;
                    if (dx > 0) newX += step;
                    else if (dx < 0) newX -= step;
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newX = position.x;
                    }
                }
            }
        }

        if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
            newX = position.x;
            newY = position.y;
        }
    }
    else {
        // 巡逻模式
        directionChangeTimer -= deltaTime;
        if (directionChangeTimer <= 0.0f || currentDirection == NONE || !CanMoveInDirection(mazeGen, currentDirection, cellSize)) {
            currentDirection = GetRandomValidDirection(mazeGen, cellSize);
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(minDirectionChangeInterval, maxDirectionChangeInterval);
            directionChangeTimer = dis(gen);
        }

        if (currentDirection != NONE) {
            switch (currentDirection) {
            case UP:    newY -= step; break;
            case RIGHT: newX += step; break;
            case DOWN:  newY += step; break;
            case LEFT:  newX -= step; break;
            }

            if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                newX = position.x;
                newY = position.y;
                directionChangeTimer = 0.0f;
            }
        }
    }

    position.x = newX;
    position.y = newY;
}

// --- 视线检测 (LOS) ---
bool Monster::HasLineOfSight(const MazeGenerator& mazeGen, float startX, float startY, float endX, float endY, float cellSize) const {
    float dx = endX - startX;
    float dy = endY - startY;
    int steps = static_cast<int>(std::ceil(std::max(std::abs(dx), std::abs(dy)) / cellSize));
    if (steps == 0) steps = 1;

    float xInc = dx / steps;
    float yInc = dy / steps;
    float x = startX, y = startY;

    for (int i = 0; i <= steps; ++i) {
        int gx = static_cast<int>(std::floor(x / cellSize));
        int gy = static_cast<int>(std::floor(y / cellSize));

        if (gx < 0 || gx >= mazeGen.width || gy < 0 || gy >= mazeGen.height) {
            return false;
        }

        const Cell& cell = mazeGen.maze[gy][gx];
        float left = gx * cellSize;
        float right = (gx + 1) * cellSize;
        float top = gy * cellSize;
        float bottom = (gy + 1) * cellSize;

        // 检查垂直墙（左右）
        if (xInc > 0 && gx + 1 < mazeGen.width) {
            float t = (right - x) / xInc;
            if (t >= 0 && t <= 1) {
                float iy = y + t * yInc;
                if (iy >= top && iy <= bottom && cell.walls[1]) return false;
            }
        }
        else if (xInc < 0 && gx - 1 >= 0) {
            float t = (left - x) / xInc;
            if (t >= 0 && t <= 1) {
                float iy = y + t * yInc;
                if (iy >= top && iy <= bottom && cell.walls[3]) return false;
            }
        }

        // 检查水平墙（上下）
        if (yInc > 0 && gy + 1 < mazeGen.height) {
            float t = (bottom - y) / yInc;
            if (t >= 0 && t <= 1) {
                float ix = x + t * xInc;
                if (ix >= left && ix <= right && cell.walls[2]) return false;
            }
        }
        else if (yInc < 0 && gy - 1 >= 0) {
            float t = (top - y) / yInc;
            if (t >= 0 && t <= 1) {
                float ix = x + t * xInc;
                if (ix >= left && ix <= right && cell.walls[0]) return false;
            }
        }

        x += xInc;
        y += yInc;
    }

    return true;
}