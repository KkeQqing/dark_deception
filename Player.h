#define GLM_ENABLE_EXPERIMENTAL
#pragma once
#include <glm/glm.hpp>
#include "MazeGenerator.h" // 需要访问迷宫结构

// 玩家类
class Player {
public:
	glm::vec2 position; // 玩家位置
	float radius = 10.0f; // 玩家半径
	float speed = 150.0f; // 玩家速度
	bool isAccelerating = false; // 是否在加速
	float accelTimer = 0.0f; // 加速计时器
	float cooldownE = 0.0f; // 技能E冷却时间
	float cooldownQ = 0.0f; // 技能Q冷却时间

	Player(float x, float y); // 构造函数声明

	// 更新玩家状态
	void Update(float deltaTime);

	// 检测玩家是否在指定单元格内
	bool IsInCell(int cx, int cy, float cellSize) const;

	// --- 新增: 精确墙体碰撞检测 ---
	bool CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const;
};