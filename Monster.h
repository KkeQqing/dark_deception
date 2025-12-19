#pragma once
#include <glm/glm.hpp>
#include <vector> // For path storage and A*
#include <queue>  // For priority_queue in A*
#include <functional> // For std::function
#include "MazeGenerator.h" // Need access to maze structure

class Player; // Forward declaration

enum class MonsterState {
    PATROLLING,
    CHASING
};

// Monster class
class Monster {
public:
    glm::vec2 position; // Monster position
    float radius = 10.0f; // Monster radius
    float baseSpeed = 80.0f; // Base speed
    float currentSpeed = 80.0f; // Current speed
    std::vector<glm::vec2> path; // Path to follow (for pathfinding)
    size_t currentPathIndex = 0; // Index of the next node in the path
    bool frozen = false; // Is frozen
    bool visible = true; // Is visible
    float detectionRange = 200.0f; // Detection range
    float chaseSpeed = 150.0f; // Chase speed
    float stuckTimer = 0.0f; // 用于记录怪物是否卡住的时间
    MonsterState state = MonsterState::PATROLLING; // 当前状态

    // --- Enhanced Patrol ---
    glm::vec2 homePosition; // Patrol center point
    float patrolRadius = 100.0f; // Patrol radius

    // --- 新增：方向控制 ---
    enum Direction { NONE = -1, UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 };
    Direction currentDirection = NONE; // 当前移动方向
    float directionChangeTimer = 0.0f; // 方向改变定时器
    const float minDirectionChangeInterval = 0.5f; // 最短方向改变间隔
    const float maxDirectionChangeInterval = 2.0f; // 最长方向改变间隔

    Monster(float x, float y); // Constructor declaration

    // Update monster state
    // Pass MazeGenerator reference for pathfinding/collision
    void Update(float deltaTime, const Player& player, const MazeGenerator& mazeGen, float cellSize);

private:
    // --- Pathfinding Structures (if used) ---
    struct Node {
        int x, y;
        float g, h, f; // g: cost from start, h: heuristic to goal, f: total cost (g+h)
        Node* parent;
        bool operator>(const Node& other) const { return f > other.f; } // For min-heap based on 'f'
    };


    // --- Helper Functions ---
    // Checks if the monster's circle at (newX, newY) collides with walls in its current cell.
    bool CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const;

    // Determines a new valid random direction for patrolling
    Direction GetRandomValidDirection(const MazeGenerator& mazeGen, float cellSize) const;

    // Checks if moving one step in the given direction from the current position is possible
    bool CanMoveInDirection(const MazeGenerator& mazeGen, Direction dir, float cellSize) const;

    // --- 新增: Line-of-Sight (LOS) Check ---
    // Checks if a line segment from (startX, startY) to (endX, endY) intersects any wall in the maze.
    bool HasLineOfSight(const MazeGenerator& mazeGen, float startX, float startY, float endX, float endY, float cellSize) const;

    // --- Pathfinding Functions (if used) ---
    // Calculates Manhattan distance heuristic between two points (grid coordinates).
    float Heuristic(int x1, int y1, int x2, int y2) const;
    // Finds a path from current monster position to target using A* (or simpler method)
    bool FindPath(const MazeGenerator& mazeGen, float cellSize, const glm::vec2& targetPos);
    // Checks if moving from (fromX, fromY) to (toX, toY) crosses a wall in the maze.
    bool IsPathBlockedByWall(const MazeGenerator& mazeGen, float fromX, float fromY, float toX, float toY, float cellSize) const;
};