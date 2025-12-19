#define GLM_ENABLE_EXPERIMENTAL
#include "Monster.h"
#include "Player.h" // 包含 Player 定义
#include <cmath>   // For sqrt, M_PI
#include <glm/gtx/norm.hpp> // For glm::distance2 or glm::length2 if needed
#define M_PI 3.14159265358979323846

// 构造函数实现
Monster::Monster(float x, float y)
    : position(x, y), targetPosition(x, y),
    homePosition(x, y), // 设置巡逻中心点
    rng(std::random_device{}()), dist(-1.0f, 1.0f) // 初始化随机数生成器
{
}

// 更新怪物状态
void Monster::Update(float deltaTime, const Player& player) {
    if (frozen) {
        // 冻结状态下不移动
        return;
    }

    float dx = player.position.x - position.x;
    float dy = player.position.y - position.y;
    float distanceToPlayer = sqrt(dx * dx + dy * dy);

    if (distanceToPlayer < detectionRange) {
        // 进入追逐模式
        currentSpeed = chaseSpeed;
        visible = true;
        // Move towards player
        if (distanceToPlayer > 0) {
            dx /= distanceToPlayer; dy /= distanceToPlayer;
            position.x += dx * currentSpeed * deltaTime;
            position.y += dy * currentSpeed * deltaTime;
        }
    }
    else {
        // 恢复正常巡逻速度和可见性判断
        currentSpeed = baseSpeed;
        visible = (distanceToPlayer < 150.0f); // 距离较近时可见

        // --- 增强巡逻逻辑 ---
        // 检查是否到达目标点或随机决定是否选择新目标
        float distToTarget = glm::distance(position, targetPosition);
        // 如果很接近目标点，或者有一定几率（例如每帧约1%），则选择新目标
        if (distToTarget < 5.0f || (rng() % 100) < 1) {
            // 选择一个新的巡逻目标点，在 homePosition 附近
            float angle = dist(rng) * 2.0f * static_cast<float>(M_PI); // 随机角度
            float radius = dist(rng) * patrolRadius; // 随机半径 (0 到 patrolRadius)
            targetPosition.x = homePosition.x + radius * cos(angle);
            targetPosition.y = homePosition.y + radius * sin(angle);

            // 简单的边界检查，防止目标点离家太远 (可根据需要加强)
            targetPosition.x = glm::clamp(targetPosition.x, homePosition.x - patrolRadius, homePosition.x + patrolRadius);
            targetPosition.y = glm::clamp(targetPosition.y, homePosition.y - patrolRadius, homePosition.y + patrolRadius);
        }

        // 向目标点移动
        dx = targetPosition.x - position.x;
        dy = targetPosition.y - position.y;
        float distToTargetFull = sqrt(dx * dx + dy * dy);
        if (distToTargetFull > 0) {
            dx /= distToTargetFull; dy /= distToTargetFull;
            position.x += dx * currentSpeed * deltaTime;
            position.y += dy * currentSpeed * deltaTime;
        }
    }
}