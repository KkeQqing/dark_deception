#pragma once
#include <glm/glm.hpp>
#include <vector> // 用于存储路径和 A* 算法
#include <queue>  // 用于 A* 中的优先队列 (priority_queue)
#include <functional> // 用于 std::function
#include "MazeGenerator.h" // 需要访问迷宫结构体

class Player; // 前向声明

// 怪物状态枚举
enum class MonsterState {
    PATROLLING, // 巡逻
    CHASING     // 追逐
};

// Monster 类
class Monster {
public:
    glm::vec2 position; // 怪物位置
    float radius = 10.0f; // 怪物半径
    float baseSpeed = 80.0f; // 基础速度
    float currentSpeed = 80.0f; // 当前速度
    std::vector<glm::vec2> path; // 要跟随的路径 (用于寻路)
    size_t currentPathIndex = 0; // 路径中下一个节点的索引
    bool frozen = false; // 是否被冻结
    bool visible = true; // 是否可见
    float detectionRange = 200.0f; // 探测范围
    float chaseSpeed = 150.0f; // 追逐速度
    float stuckTimer = 0.0f; // 用于记录怪物是否卡住的时间
    MonsterState state = MonsterState::PATROLLING; // 当前状态

    // --- 增强巡逻 ---
    glm::vec2 homePosition; // 巡逻中心点
    float patrolRadius = 100.0f; // 巡逻半径

    // --- 新增：方向控制 ---
    enum Direction { NONE = -1, UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 };
    Direction currentDirection = NONE; // 当前移动方向
    float directionChangeTimer = 0.0f; // 方向改变定时器
    const float minDirectionChangeInterval = 0.5f; // 最短方向改变间隔 (秒)
    const float maxDirectionChangeInterval = 2.0f; // 最长方向改变间隔 (秒)

    // 构造函数声明
    Monster(float x, float y);

    // 更新怪物状态
    // 传递 MazeGenerator 引用用于寻路/碰撞检测
    void Update(float deltaTime, const Player& player, const MazeGenerator& mazeGen, float cellSize);

private:
    // --- 寻路结构体 (如果使用) ---
    struct Node {
        int x, y;
        float g, h, f; // g: 从起点到当前点的实际成本, h: 到目标点的启发式估算成本 (Manhattan distance), f: 总成本 (g+h)
        Node* parent;
        // 为了基于 'f' 值构建最小堆 (min-heap)，重载大于运算符
        bool operator>(const Node& other) const { return f > other.f; }
    };

    // --- 辅助函数 ---

    // 检查位于 (newX, newY) 的怪物圆形区域是否会与其所在单元格的墙壁发生碰撞。
    bool CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const;

    // 确定一个用于巡逻的有效随机新方向
    Direction GetRandomValidDirection(const MazeGenerator& mazeGen, float cellSize) const;

    // 检查从当前位置向给定方向移动一步是否可行
    bool CanMoveInDirection(const MazeGenerator& mazeGen, Direction dir, float cellSize) const;

    // --- 新增: 视线检测 (Line-of-Sight, LOS) ---
    // 检查从 (startX, startY) 到 (endX, endY) 的线段是否与迷宫中的任何墙壁相交。
    bool HasLineOfSight(const MazeGenerator& mazeGen, float startX, float startY, float endX, float endY, float cellSize) const;

    // --- 寻路函数 (如果使用) ---

    // 计算两点之间 (网格坐标) 的曼哈顿距离启发式值 (Manhattan distance heuristic)。
    float Heuristic(int x1, int y1, int x2, int y2) const;

    // 使用 A* (或更简单的方法) 从怪物当前位置寻找一条通往目标点的路径。
    bool FindPath(const MazeGenerator& mazeGen, float cellSize, const glm::vec2& targetPos);

    // 检查从 (fromX, fromY) 移动到 (toX, toY) 是否会穿过迷宫中的一堵墙。
    bool IsPathBlockedByWall(const MazeGenerator& mazeGen, float fromX, float fromY, float toX, float toY, float cellSize) const;
};