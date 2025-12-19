#pragma once
#include <glm/glm.hpp>
#include <vector> // For path storage and A*
#include <queue>  // For priority_queue in A*
#include <functional> // For std::function
#include "MazeGenerator.h" // Need access to maze structure

class Player; // Forward declaration

// Monster class
class Monster {
public:
    glm::vec2 position; // Monster position
    float radius = 12.0f; // Monster radius
    float baseSpeed = 80.0f; // Base speed
    float currentSpeed = 80.0f; // Current speed
    std::vector<glm::vec2> path; // Path to follow (for pathfinding)
    size_t currentPathIndex = 0; // Index of the next node in the path
    bool frozen = false; // Is frozen
    bool visible = true; // Is visible
    float detectionRange = 200.0f; // Detection range
    float chaseSpeed = 150.0f; // Chase speed

    // --- Enhanced Patrol ---
    glm::vec2 homePosition; // Patrol center point
    float patrolRadius = 100.0f; // Patrol radius

    Monster(float x, float y); // Constructor declaration

    // Update monster state
    // Pass MazeGenerator reference for pathfinding/collision
    void Update(float deltaTime, const Player& player, const MazeGenerator& mazeGen, float cellSize);

private:
    // --- New: Pathfinding Structures ---
    struct Node {
        int x, y;
        float g, h, f; // g: cost from start, h: heuristic to goal, f: total cost (g+h)
        Node* parent;
        bool operator>(const Node& other) const { return f > other.f; } // For min-heap based on 'f'
    };

    glm::vec2 patrolTarget; // The current random point to patrol towards
    float patrolTargetTimer = 0.0f; // Timer to decide when to pick a new patrol target

    // --- New: Pathfinding Function ---
    // Finds a path from current monster position to target using A*
    // Stores the path in the 'path' member vector.
    // Returns true if a path was found, false otherwise.
    bool FindPath(const MazeGenerator& mazeGen, float cellSize, const glm::vec2& targetPos);

    // --- New: Helper for Pathfinding/Collision ---
    // Checks if moving from (fromX, fromY) to (toX, toY) crosses a wall in the maze.
    bool IsPathBlockedByWall(const MazeGenerator& mazeGen, float fromX, float fromY, float toX, float toY, float cellSize) const;

    // --- New: Simple Wall Collision for Monster Movement ---
    // Checks if the monster's circle at (newX, newY) collides with walls in its current cell.
    bool CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const;

    // --- New: Helper for Pathfinding ---
    // Calculates Manhattan distance heuristic between two points (grid coordinates).
    float Heuristic(int x1, int y1, int x2, int y2) const;
};