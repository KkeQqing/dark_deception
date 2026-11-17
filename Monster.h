#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <random>
#include "MazeGenerator.h"

class Player; 

// 怪物状态枚举：用于表示怪物当前的行为模式
enum class MonsterState {
    PATROLLING, // 巡逻状态
    CHASING     // 追逐状态
};

class Monster {
public:
    glm::vec2 position;                 // 怪物当前位置
    float radius = 10.0f;               // 碰撞半径
    float baseSpeed = 50.0f;            // 基础移动速度
    float currentSpeed = 80.0f;         
    bool frozen = false;                // 是否被冻结
    bool visible = true;                // 是否可见
    float detectionRange = 200.0f;      // 发现玩家的最大距离
    float chaseSpeed = 150.0f;          // 追逐玩家时的速度
    float stuckTimer = 0.0f;            // 检测是否长时间无法移动
    MonsterState state = MonsterState::PATROLLING; // 当前行为状态，默认为巡逻

    glm::vec2 homePosition;             // 出生点/巡逻中心点
    float patrolRadius = 500.0f;        // 巡逻范围半径（怪物不会离开该区域太远）

    // 方向枚举
    enum Direction {
        NONE = -1,  // 无方向（初始或停止状态）
        UP = 0,     // 上
        RIGHT = 1,  // 右
        DOWN = 2,   // 下
        LEFT = 3    // 左
    };
    Direction currentDirection = NONE;  // 当前移动方向
    float directionChangeTimer = 0.0f;  // 方向切换计时器
    const float minDirectionChangeInterval = 0.5f;  // 最短方向切换间隔（秒）
    const float maxDirectionChangeInterval = 2.0f;  // 最长方向切换间隔（秒）

    Monster(float x, float y);

    // 每帧更新逻辑
    // 参数：
    //   deltaTime: 帧时间间隔（秒）
    //   player: 玩家对象的常量引用（用于检测距离和视线）
    //   mazeGen: 迷宫生成器（用于碰撞检测和路径判断）
    //   cellSize: 迷宫每个格子的世界尺寸（像素或单位长度）
    void Update(float deltaTime, const Player& player, const MazeGenerator& mazeGen, float cellSize);

private:
    // 检测在指定新位置 (newX, newY) 是否会与墙壁发生碰撞
    // 返回 true 表示有碰撞（不能移动），false 表示可以移动
    bool CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const;

    // 在当前所在迷宫格子中，随机选择一个合法的移动方向（不撞墙）
    // 如果没有合法方向，则返回 NONE
    Direction GetRandomValidDirection(const MazeGenerator& mazeGen, float cellSize) const;

    // 判断在给定方向上是否可以移动（即前方不是墙）
    bool CanMoveInDirection(const MazeGenerator& mazeGen, Direction dir, float cellSize) const;

    // 检查从起点 (startX, startY) 到终点 (endX, endY) 是否存在无障碍视线（用于判断是否“看到”玩家）
    // 使用射线检测或逐格检查方式实现
    bool HasLineOfSight(const MazeGenerator& mazeGen, float startX, float startY, float endX, float endY, float cellSize) const;
};