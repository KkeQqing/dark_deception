#pragma once
#include <glm/glm.hpp>
#include "Player.h"

// 怪物类
class Monster {
public:
	glm::vec2 position; // 怪物位置
	float radius = 6.0f; // 怪物半径
	float baseSpeed = 80.0f; // 基础速度
	float currentSpeed = 80.0f; // 当前速度
	glm::vec2 targetPosition; // 目标位置
	bool frozen = false; // 是否被冻结
	bool visible = true; // 是否可见
	float detectionRange = 200.0f; // 侦测范围
	float chaseSpeed = 150.0f; // 追逐速度

    Monster(float x, float y) : position(x, y), targetPosition(x, y) {}

	// 更新怪物状态
    void Update(float deltaTime, const Player& player) {
        if (frozen) return;

        float dx = player.position.x - position.x;
        float dy = player.position.y - position.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < detectionRange) {
            currentSpeed = chaseSpeed;
            visible = true;
            // Move towards player
            if (distance > 0) {
                dx /= distance; dy /= distance;
                position.x += dx * currentSpeed * deltaTime;
                position.y += dy * currentSpeed * deltaTime;
            }
        }
        else {
            currentSpeed = baseSpeed;
            // Simple patrol logic placeholder
            visible = (distance < 150.0f);
        }
    }
};