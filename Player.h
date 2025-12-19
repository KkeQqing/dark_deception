#pragma once
#include <glm/glm.hpp>

// 玩家类
class Player {
public:
	glm::vec2 position; // 玩家位置
	float radius = 8.0f; // 玩家半径
	float speed = 150.0f; // 玩家速度
	bool isAccelerating = false; // 是否在加速
	float accelTimer = 0.0f; // 加速计时器
	float cooldownE = 0.0f; // 技能E冷却时间
	float cooldownQ = 0.0f; // 技能Q冷却时间

	Player(float x, float y) : position(x, y) {}// 构造函数

	// 更新玩家状态
    void Update(float deltaTime) {
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
    bool IsInCell(int cx, int cy, float cellSize) const {
        float left = cx * cellSize;
        float right = (cx + 1) * cellSize;
        float top = cy * cellSize;
        float bottom = (cy + 1) * cellSize;
        return (position.x + radius > left && position.x - radius < right &&
            position.y + radius > top && position.y - radius < bottom);
    }
};