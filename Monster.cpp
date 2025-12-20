#include "Monster.h"
#include "Player.h" // Include Player definition
#include <cmath>   // For sqrt, abs, floor
#include <algorithm> // For std::reverse
#include <climits> // For INT_MAX
#include <cstring> // For memset (optional, but useful for resetting arrays)
#include <random>  // For random number generation
#include <glm/gtc/constants.hpp> // 可能需要这个头文件用于 PI

// --- Helper: Simple Wall Collision for Monster Movement ---
bool Monster::CheckWallCollision(const MazeGenerator& mazeGen, float newX, float newY, float cellSize) const {
    // Calculate the grid cell the monster's center is in
    int gridX = static_cast<int>(floor(newX / cellSize));
    int gridY = static_cast<int>(floor(newY / cellSize));

    // Boundary check (assume walls outside the maze)
    if (gridX < 0 || gridX >= mazeGen.width || gridY < 0 || gridY >= mazeGen.height) {
        return true; // Hit wall
    }

    const Cell& currentCell = mazeGen.maze[gridY][gridX];

    // Check distances against cell boundaries
    float cellLeft = gridX * cellSize;
    float cellRight = (gridX + 1) * cellSize;
    float cellTop = gridY * cellSize;
    float cellBottom = (gridY + 1) * cellSize;

    // Monster boundaries
    float monsterLeft = newX - radius;
    float monsterRight = newX + radius;
    float monsterTop = newY - radius;
    float monsterBottom = newY + radius;

    // If monster boundary exceeds cell boundary and there's a wall, collision occurs
    if (currentCell.walls[3] && monsterLeft < cellLeft + 1.0f) return true; // Left wall (add small buffer)
    if (currentCell.walls[1] && monsterRight > cellRight - 1.0f) return true; // Right wall
    if (currentCell.walls[0] && monsterTop < cellTop + 1.0f) return true; // Top wall
    if (currentCell.walls[2] && monsterBottom > cellBottom - 1.0f) return true; // Bottom wall

    return false; // No collision
}

// --- Helper: Check if can move one step in a given direction ---
bool Monster::CanMoveInDirection(const MazeGenerator& mazeGen, Direction dir, float cellSize) const {
    float testX = position.x;
    float testY = position.y;
    float stepDistance = currentSpeed * (1.0f / 60.0f); // Estimate a small step (assuming ~60 FPS)

    switch (dir) {
    case UP:    testY -= stepDistance; break;
    case RIGHT: testX += stepDistance; break;
    case DOWN:  testY += stepDistance; break;
    case LEFT:  testX -= stepDistance; break;
    default: return false; // NONE or invalid
    }

    // Check collision at the new potential position
    return !CheckWallCollision(mazeGen, testX, testY, cellSize);
}

// --- Helper: Get a random valid direction for patrolling ---
Monster::Direction Monster::GetRandomValidDirection(const MazeGenerator& mazeGen, float cellSize) const {
    std::vector<Direction> validDirections;
    // Check all four directions
    for (int d = UP; d <= LEFT; ++d) {
        Direction dir = static_cast<Direction>(d);
        if (CanMoveInDirection(mazeGen, dir, cellSize)) {
            validDirections.push_back(dir);
        }
    }

    if (!validDirections.empty()) {
        // Pick a random valid direction
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, validDirections.size() - 1);
        return validDirections[dis(gen)];
    }
    return NONE; // No valid direction found
}

// --- Constructor implementation ---
Monster::Monster(float x, float y)
    : position(x, y),
    homePosition(x, y) // Set patrol center point
{
    // Initialize path index
    currentPathIndex = 0;
    // Initialize direction randomly
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(minDirectionChangeInterval, maxDirectionChangeInterval);
    directionChangeTimer = dis(gen); // Start with a random timer
}

// --- Update monster state ---
void Monster::Update(float deltaTime, const Player& player, const MazeGenerator& mazeGen, float cellSize) {
    if (frozen) {
        // Frozen state: do not move
        return;
    }

    float dx_to_player = player.position.x - position.x;
    float dy_to_player = player.position.y - position.y;
    float distanceToPlayer = sqrt(dx_to_player * dx_to_player + dy_to_player * dy_to_player);

    // --- 修改: Determine state with Line-of-Sight check ---
    // Check if player is within detection range AND has unobstructed line of sight
    bool playerInRange = distanceToPlayer < detectionRange;
    bool playerVisible = false;
    if (playerInRange) {
        playerVisible = HasLineOfSight(mazeGen, position.x, position.y, player.position.x, player.position.y, cellSize);
    }

    if (playerInRange && playerVisible) {
        state = MonsterState::CHASING;
        currentSpeed = chaseSpeed;
        visible = true;
    }
    else {
        state = MonsterState::PATROLLING;
        currentSpeed = baseSpeed;
        // Optional: Make monster less visible when far away even if technically in range but not seen
        visible = (distanceToPlayer < 150.0f);
    }


    // --- Movement Logic ---
    float stepDistance = currentSpeed * deltaTime;
    float newX = position.x;
    float newY = position.y;

    if (state == MonsterState::CHASING) {
        // --- Chase Mode: Move towards player ---
        // Prioritize the axis with the larger distance component
        if (abs(dx_to_player) > abs(dy_to_player)) {
            // Try horizontal movement first
            if (dx_to_player > 0) {
                newX += stepDistance; // Move right
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newX = position.x; // Revert if collision
                    // Then try vertical
                    if (dy_to_player > 0) newY += stepDistance;
                    else if (dy_to_player < 0) newY -= stepDistance;
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newY = position.y; // Revert if collision
                    }
                }
            }
            else if (dx_to_player < 0) {
                newX -= stepDistance; // Move left
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newX = position.x; // Revert if collision
                    // Then try vertical
                    if (dy_to_player > 0) newY += stepDistance;
                    else if (dy_to_player < 0) newY -= stepDistance;
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newY = position.y; // Revert if collision
                    }
                }
            }
        }
        else {
            // Try vertical movement first
            if (dy_to_player > 0) {
                newY += stepDistance; // Move down
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newY = position.y; // Revert if collision
                    // Then try horizontal
                    if (dx_to_player > 0) newX += stepDistance;
                    else if (dx_to_player < 0) newX -= stepDistance;
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newX = position.x; // Revert if collision
                    }
                }
            }
            else if (dy_to_player < 0) {
                newY -= stepDistance; // Move up
                if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    newY = position.y; // Revert if collision
                    // Then try horizontal
                    if (dx_to_player > 0) newX += stepDistance;
                    else if (dx_to_player < 0) newX -= stepDistance;
                    if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        newX = position.x; // Revert if collision
                    }
                }
            }
        }
        // Final check in case combined movement failed
        if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
            newX = position.x;
            newY = position.y;
        }

    }
    else { // PATROLLING
        // --- Patrol Mode: Move in a preferred direction ---
        directionChangeTimer -= deltaTime;

        // Change direction if timer expired or current direction is blocked
        if (directionChangeTimer <= 0.0f || currentDirection == NONE || !CanMoveInDirection(mazeGen, currentDirection, cellSize)) {
            currentDirection = GetRandomValidDirection(mazeGen, cellSize);
            // Reset timer for next direction change
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(minDirectionChangeInterval, maxDirectionChangeInterval);
            directionChangeTimer = dis(gen);
        }

        // Execute movement based on current direction
        if (currentDirection != NONE) {
            switch (currentDirection) {
            case UP:    newY -= stepDistance; break;
            case RIGHT: newX += stepDistance; break;
            case DOWN:  newY += stepDistance; break;
            case LEFT:  newX -= stepDistance; break;
            }

            // Check for collision after attempting move
            if (CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                // Revert movement if collision occurred
                newX = position.x;
                newY = position.y;
                // Force a direction change next update
                directionChangeTimer = 0.0f;
            }
        }
    }

    // Apply final calculated position
    position.x = newX;
    position.y = newY;
}

// --- Placeholder implementations for unused pathfinding functions ---
// These can be removed or kept empty if not using A* pathfinding currently.
float Monster::Heuristic(int x1, int y1, int x2, int y2) const {
    return static_cast<float>(abs(x1 - x2) + abs(y1 - y2)); // Manhattan distance
}

bool Monster::IsPathBlockedByWall(const MazeGenerator& /*mazeGen*/, float /*fromX*/, float /*fromY*/, float /*toX*/, float /*toY*/, float /*cellSize*/) const {
    // Placeholder, not used in this revised logic
    return false;
}

bool Monster::FindPath(const MazeGenerator& /*mazeGen*/, float /*cellSize*/, const glm::vec2& /*targetPos*/) {
    // Placeholder, not used in this revised logic
    path.clear();
    return false; // Indicate pathfinding not used/failed
}

// --- 新增: Line-of-Sight (LOS) Check Implementation ---
bool Monster::HasLineOfSight(const MazeGenerator& mazeGen, float startX, float startY, float endX, float endY, float cellSize) const {
    // DDA Algorithm adapted for grid-based collision checking

    float dx = endX - startX;
    float dy = endY - startY;
    int steps = static_cast<int>(std::ceil(std::max(std::abs(dx), std::abs(dy)) / cellSize)); // Estimate steps based on grid size

    if (steps == 0) steps = 1; // Ensure we check at least the starting cell

    float xIncrement = dx / steps;
    float yIncrement = dy / steps;

    float currentX = startX;
    float currentY = startY;

    for (int i = 0; i <= steps; ++i) {
        // Calculate the grid cell containing the current point
        int gridX = static_cast<int>(std::floor(currentX / cellSize));
        int gridY = static_cast<int>(std::floor(currentY / cellSize));

        // Boundary check
        if (gridX < 0 || gridX >= mazeGen.width || gridY < 0 || gridY >= mazeGen.height) {
            // Treat area outside maze as blocked
            return false;
        }

        const Cell& cell = mazeGen.maze[gridY][gridX];

        // Define cell boundaries
        float cellLeft = gridX * cellSize;
        float cellRight = (gridX + 1) * cellSize;
        float cellTop = gridY * cellSize;
        float cellBottom = (gridY + 1) * cellSize;

        // --- Check if the ray segment within this cell crosses any wall ---

        // 1. Check for vertical wall crossing (left/right)
        if (xIncrement > 0) { // Ray moving right
            // Potential intersection with left wall of next cell
            if (gridX + 1 < mazeGen.width) {
                float t = (cellRight - currentX) / xIncrement;
                if (t >= 0 && t <= 1) { // Intersection point is on the ray segment
                    float intersectY = currentY + t * yIncrement;
                    // Check if intersection point is within the vertical span of the wall
                    if (intersectY >= cellTop && intersectY <= cellBottom) {
                        // Check if the wall exists
                        if (mazeGen.maze[gridY][gridX].walls[1]) { // Right wall of current cell
                            return false; // Blocked by right wall
                        }
                    }
                }
            }
        }
        else if (xIncrement < 0) { // Ray moving left
            // Potential intersection with right wall of previous cell
            if (gridX - 1 >= 0) {
                float t = (cellLeft - currentX) / xIncrement;
                if (t >= 0 && t <= 1) {
                    float intersectY = currentY + t * yIncrement;
                    if (intersectY >= cellTop && intersectY <= cellBottom) {
                        if (mazeGen.maze[gridY][gridX].walls[3]) { // Left wall of current cell
                            return false; // Blocked by left wall
                        }
                    }
                }
            }
        }

        // 2. Check for horizontal wall crossing (top/bottom)
        if (yIncrement > 0) { // Ray moving down
            // Potential intersection with top wall of next cell
            if (gridY + 1 < mazeGen.height) {
                float t = (cellBottom - currentY) / yIncrement;
                if (t >= 0 && t <= 1) {
                    float intersectX = currentX + t * xIncrement;
                    if (intersectX >= cellLeft && intersectX <= cellRight) {
                        if (mazeGen.maze[gridY][gridX].walls[2]) { // Bottom wall of current cell
                            return false; // Blocked by bottom wall
                        }
                    }
                }
            }
        }
        else if (yIncrement < 0) { // Ray moving up
            // Potential intersection with bottom wall of previous cell
            if (gridY - 1 >= 0) {
                float t = (cellTop - currentY) / yIncrement;
                if (t >= 0 && t <= 1) {
                    float intersectX = currentX + t * xIncrement;
                    if (intersectX >= cellLeft && intersectX <= cellRight) {
                        if (mazeGen.maze[gridY][gridX].walls[0]) { // Top wall of current cell
                            return false; // Blocked by top wall
                        }
                    }
                }
            }
        }

        // Move to the next sample point along the ray
        currentX += xIncrement;
        currentY += yIncrement;
    }

    // If no walls were hit during the traversal, LOS is clear
    return true;
}