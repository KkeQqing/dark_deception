#pragma once
#include <glm/glm.hpp>

// 收集物类
class Collectible {
public:
	glm::vec2 position;// 位置 
	float size = 12.0f; // 大小
	bool collected = false; // 是否已被收集

    Collectible(float x, float y) : position(x, y) {}
};