#define GLM_ENABLE_EXPERIMENTAL
#pragma once
#include <glm/glm.hpp>
#include <random> // 添加随机数支持

class Player; // 前向声明

// 怪物类
class Monster {
public:
	glm::vec2 position; // 怪物位置
	float radius = 12.0f; // 怪物半径
	float baseSpeed = 80.0f; // 基础速度
	float currentSpeed = 80.0f; // 当前速度
	glm::vec2 targetPosition; // 目标位置 (用于巡逻)
	bool frozen = false; // 是否被冻结
	bool visible = true; // 是否可见
	float detectionRange = 200.0f; // 侦测范围
	float chaseSpeed = 150.0f; // 追逐速度

	// --- 新增: 复杂巡逻 ---
	glm::vec2 homePosition; // 巡逻的中心点
	float patrolRadius = 100.0f; // 巡逻半径
	std::mt19937 rng; // 每个怪物有自己的随机数生成器
	std::uniform_real_distribution<float> dist; // 分布 [-1, 1]

	Monster(float x, float y); // 构造函数声明

	// 更新怪物状态
	void Update(float deltaTime, const Player& player);
};