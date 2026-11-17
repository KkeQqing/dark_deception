#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "MazeGenerator.h" 

// 玩家类
class Player {
public:
    glm::vec2 position; // 玩家位置 
    float radius = 8.0f; // 玩家半径
    bool isAccelerating = false; // 是否在加速
    float accelTimer = 0.0f; // 加速计时器
    float cooldownE = 0.0f; // 技能E冷却时间
    float cooldownQ = 0.0f; // 技能Q冷却时间
    int cellX = 0;       // 当前所在的列索引
    int cellY = 0;       // 当前所在的行索引
    glm::vec2 targetPosition; // 移动位置
    bool isMoving = false;    // 是否正在进行移动
    float moveSpeed = 200.0f; // 移动速度

    Player(int startX, int startY, float cellSize); // 构造函数声明，接受起始单元格和单元格大小

    // --- 方法 ---
    // 更新玩家状态 (移动动画、技能等)
    void Update(float deltaTime);

    // 检测玩家是否在指定单元格内 (可能不再需要，但保留以防万一)
    bool IsInCell(int cx, int cy, float cellSize) const;

    // --- 新增方法：处理基于单元格的移动 ---
    // 检查是否可以向某个方向移动
    bool CanMoveTo(int newCellX, int newCellY, const MazeGenerator& mazeGen) const;

    // 触发向指定方向的移动 (如果可能的话)
    void TryMove(int dx, int dy, const MazeGenerator& mazeGen, float cellSize);

    // 执行移动动画
    void PerformMovement(float deltaTime, float cellSize);

};