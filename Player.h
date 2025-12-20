#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "MazeGenerator.h" // 需要访问迷宫结构

// 玩家类
class Player {
public:
    // --- 原有成员保持不变 ---
    glm::vec2 position; // 玩家精确位置 (像素)，用于渲染
    float radius = 8.0f; // 玩家半径
    float speed = 150.0f; // 玩家基础移动速度 (像素/秒)
    bool isAccelerating = false; // 是否在加速
    float accelTimer = 0.0f; // 加速计时器
    float cooldownE = 0.0f; // 技能E冷却时间
    float cooldownQ = 0.0f; // 技能Q冷却时间

    // --- 新增成员：用于基于单元格的移动 ---
    int cellX = 0;       // 玩家当前所在的列索引
    int cellY = 0;       // 玩家当前所在的行索引
    glm::vec2 targetPosition; // 移动的目标像素位置 (通常是下一个单元格中心)
    bool isMoving = false;    // 标记玩家是否正在进行移动动画
    float moveSpeed = 200.0f; // 单元格间移动的速度 (像素/秒)，可以独立于speed调整动画快慢

    // --- 构造函数 ---
    // 修改构造函数以初始化cellX, cellY和position
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

    // --- 原有的精确墙体碰撞检测 (可能不再直接用于移动判断，但在某些情况如推拉物体时可能有用) ---
    bool CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const;
};