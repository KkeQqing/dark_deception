#include "Monster.h"
#include "Player.h" // Include Player definition
#include <cmath>   // For sqrt, abs, floor
#include <algorithm> // For std::reverse
#include <climits> // For INT_MAX
#include <cstring> // For memset (optional, but useful for resetting arrays)

// Monster.cpp (靠近文件顶部，在 #include 之后)
#include <glm/gtc/constants.hpp> // 可能需要这个头文件用于 PI

// --- New: Raycasting for Line of Sight ---
// Checks if there's a direct line of sight between monster and player, considering walls.
bool HasLineOfSight(const Monster& monster, const Player& player, const MazeGenerator& mazeGen, float cellSize) {
    glm::vec2 start = monster.position;
    glm::vec2 end = player.position;
    glm::vec2 direction = end - start;
    float distance = glm::length(direction);

    if (distance < 0.001f) return true; // Avoid division by zero if positions are very close

    direction = glm::normalize(direction);

    // Simple ray marching: move along the ray in small steps
    const float stepSize = 2.0f; // Adjust step size for accuracy/performance trade-off
    float t = 0.0f; // Parameter along the ray (0=start, 1=end)

    while (t * distance < distance) { // March until we reach the player or hit a wall
        glm::vec2 testPoint = start + direction * (t * distance);

        // Check if this point hits a wall
        int gridX = static_cast<int>(floor(testPoint.x / cellSize));
        int gridY = static_cast<int>(floor(testPoint.y / cellSize));

        // Boundary check
        if (gridX < 0 || gridX >= mazeGen.width || gridY < 0 || gridY >= mazeGen.height) {
            // Treat out-of-bounds as walls
            return false;
        }

        const Cell& cell = mazeGen.maze[gridY][gridX];

        // Check if the test point is inside a wall area of the cell
        float cellLeft = gridX * cellSize;
        float cellRight = (gridX + 1) * cellSize;
        float cellTop = gridY * cellSize;
        float cellBottom = (gridY + 1) * cellSize;

        // Check against each wall of the cell
        if (cell.walls[0] && testPoint.y <= cellTop + 1.0f) return false; // Top wall
        if (cell.walls[1] && testPoint.x >= cellRight - 1.0f) return false; // Right wall
        if (cell.walls[2] && testPoint.y >= cellBottom - 1.0f) return false; // Bottom wall
        if (cell.walls[3] && testPoint.x <= cellLeft + 1.0f) return false; // Left wall

        t += stepSize / distance; // Increment parameter

        // Prevent infinite loop in case of precision issues
        if (t > 1.1f) break;
    }

    // If we got here, we didn't hit a wall
    return true;
}

// Constructor implementation
Monster::Monster(float x, float y)
    : position(x, y),
    homePosition(x, y) // Set patrol center point
{
    // Initialize path index
    currentPathIndex = 0;
}

// Update monster state
void Monster::Update(float deltaTime, const Player& player, const MazeGenerator& mazeGen, float cellSize) {
    if (frozen) {
        // Frozen state: do not move
        return;
    }

    float dx_to_player = player.position.x - position.x;
    float dy_to_player = player.position.y - position.y;
    float distanceToPlayer = sqrt(dx_to_player * dx_to_player + dy_to_player * dy_to_player);

    if (distanceToPlayer < detectionRange && HasLineOfSight(*this, player, mazeGen, cellSize)) {
        // Enter chase mode
        currentSpeed = chaseSpeed;
        visible = true;

        // --- Use Pathfinding for Chasing ---
        // Recalculate path periodically or when necessary
        // Here, we recalculate every frame for simplicity, but you might want to optimize this later
        if (FindPath(mazeGen, cellSize, player.position)) {
            // Path found, reset index to start following the new path
            currentPathIndex = 0;
        }

        // Follow the path if it exists and has nodes
        if (!path.empty() && currentPathIndex < path.size()) {
            glm::vec2 targetNode = path[currentPathIndex];
            float dx_to_node = targetNode.x - position.x;
            float dy_to_node = targetNode.y - position.y;
            float dist_to_node = sqrt(dx_to_node * dx_to_node + dy_to_node * dy_to_node);

            // If close enough to the current node, move to the next one
            if (dist_to_node < 2.0f) { // Threshold to consider reaching the node
                currentPathIndex++;
                if (currentPathIndex >= path.size()) {
                    // Reached the end of the path (player's position)
                    // Stay at the last known position or re-calculate?
                    // For now, just stop at the last node.
                    currentPathIndex = path.size() - 1;
                }
            }

            // Move towards the current node in the path
            if (currentPathIndex < path.size()) {
                targetNode = path[currentPathIndex]; // Get potentially updated node
                dx_to_node = targetNode.x - position.x;
                dy_to_node = targetNode.y - position.y;
                dist_to_node = sqrt(dx_to_node * dx_to_node + dy_to_node * dy_to_node);
                if (dist_to_node > 0) {
                    dx_to_node /= dist_to_node;
                    dy_to_node /= dist_to_node;
                    // Calculate potential new position
                    float stepDistance = currentSpeed * deltaTime;
                    float newX = position.x + dx_to_node * stepDistance;
                    float newY = position.y + dy_to_node * stepDistance;

                    // Check collision before moving
                    if (!CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                        position.x = newX;
                        position.y = newY;
                    }
                    else {
                        // If blocked by wall, try to find an alternative immediate direction
                        // This is a simple fallback, A* should ideally prevent this.
                        // You could also try moving along one axis only.
                        // For now, let's just not move in that specific direction.
                        // Or perhaps re-plan path more frequently.
                    }
                }
            }
        }
        else {
            // No valid path found, move directly towards player (old behavior) as fallback
            // This can happen if player is unreachable due to disconnected maze parts etc.
            if (distanceToPlayer > 0) {
                dx_to_player /= distanceToPlayer;
                dy_to_player /= distanceToPlayer;
                // Calculate potential new position
                float stepDistance = currentSpeed * deltaTime;
                float newX = position.x + dx_to_player * stepDistance;
                float newY = position.y + dy_to_player * stepDistance;

                // Check collision before moving
                if (!CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                    position.x = newX;
                    position.y = newY;
                }
            }
        }
    }
    else {
        // Restore normal patrol speed and visibility judgment
        currentSpeed = baseSpeed;
        visible = (distanceToPlayer < 150.0f); // Visible when closer

        // --- Random Patrol Logic ---
        // Simple state machine: PATROL_TO_TARGET -> WAIT_AT_TARGET -> Choose new target

        // If no patrol target or reached the target, choose a new random one nearby
        if (patrolTargetTimer <= 0.0f || glm::distance(position, patrolTarget) < 5.0f) {
            // Choose a new random point within patrolRadius of homePosition
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265f;
            float radius_offset = static_cast<float>(rand()) / RAND_MAX * patrolRadius;
            float targetX = homePosition.x + cos(angle) * radius_offset;
            float targetY = homePosition.y + sin(angle) * radius_offset;
            patrolTarget = glm::vec2(targetX, targetY);
            // Set timer for how long to move towards this target (e.g., 2-5 seconds)
            patrolTargetTimer = 2.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;
        }

        // Move towards the current patrol target
        float dx_patrol = patrolTarget.x - position.x;
        float dy_patrol = patrolTarget.y - position.y;
        float dist_patrol_full = sqrt(dx_patrol * dx_patrol + dy_patrol * dy_patrol);
        if (dist_patrol_full > 0) {
            dx_patrol /= dist_patrol_full;
            dy_patrol /= dist_patrol_full;
            // Calculate potential new position
            float stepDistance = currentSpeed * deltaTime;
            float newX = position.x + dx_patrol * stepDistance;
            float newY = position.y + dy_patrol * stepDistance;

            // Check collision before moving
            if (!CheckWallCollision(mazeGen, newX, newY, cellSize)) {
                position.x = newX;
                position.y = newY;
            }
            // Optional: If blocked, maybe choose a new target sooner? Or just wait?
            // For simplicity, let the timer handle re-targeting.
        }

        // Decrement the timer
        patrolTargetTimer -= deltaTime;
    }
}


// --- New: Simple Wall Collision for Monster Movement ---
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
    if (currentCell.walls[3] && monsterLeft < cellLeft) return true; // Left wall
    if (currentCell.walls[1] && monsterRight > cellRight) return true; // Right wall
    if (currentCell.walls[0] && monsterTop < cellTop) return true; // Top wall
    if (currentCell.walls[2] && monsterBottom > cellBottom) return true; // Bottom wall

    return false; // No collision
}

// --- New: Pathfinding Structures ---
// (These were declared in the header, implemented here if needed inline functions exist)

// --- New: Helper for Pathfinding ---
float Monster::Heuristic(int x1, int y1, int x2, int y2) const {
    // Using Manhattan distance as heuristic
    return static_cast<float>(abs(x1 - x2) + abs(y1 - y2));
}

// --- New: Helper for Pathfinding/Collision ---
bool Monster::IsPathBlockedByWall(const MazeGenerator& mazeGen, float fromX, float fromY, float toX, float toY, float cellSize) const {
    // Convert positions to grid coordinates
    int startX = static_cast<int>(floor(fromX / cellSize));
    int startY = static_cast<int>(floor(fromY / cellSize));
    int endX = static_cast<int>(floor(toX / cellSize));
    int endY = static_cast<int>(floor(toY / cellSize));

    // If both points are in the same cell, no wall blocking between them
    if (startX == endX && startY == endY) {
        // Check if monster itself is in that cell and hits a wall (already handled by CheckWallCollision)
        // But for path segment, if it's the same cell, likely no inter-cell wall crossing.
        // We'll assume it's fine unless monster is bigger than cell which isn't typical here.
        return false;
    }

    // --- Line Segment vs Grid Cell Intersection Logic ---
    // This is a simplified check. A more robust method would involve checking line-segment intersection with all walls.

    // Check horizontal and vertical steps needed
    int stepX = (endX > startX) ? 1 : ((endX < startX) ? -1 : 0);
    int stepY = (endY > startY) ? 1 : ((endY < startY) ? -1 : 0);

    int currentX = startX;
    int currentY = startY;

    // Determine direction vector of the line segment
    float deltaX = toX - fromX;
    float deltaY = toY - fromY;

    // Handle special cases where line is vertical or horizontal
    if (stepX == 0) { // Vertical line
        while (currentY != endY) {
            // Moving vertically, check for horizontal walls (top/bottom)
            if (currentY >= 0 && currentY < mazeGen.height && currentX >= 0 && currentX < mazeGen.width) {
                const Cell& c = mazeGen.maze[currentY][currentX];
                if ((deltaY > 0 && c.walls[2]) || (deltaY < 0 && c.walls[0])) {
                    return true; // Blocked by wall
                }
            }
            currentY += stepY;
        }
        return false; // Reached end without hitting wall
    }

    if (stepY == 0) { // Horizontal line
        while (currentX != endX) {
            // Moving horizontally, check for vertical walls (left/right)
            if (currentY >= 0 && currentY < mazeGen.height && currentX >= 0 && currentX < mazeGen.width) {
                const Cell& c = mazeGen.maze[currentY][currentX];
                if ((deltaX > 0 && c.walls[1]) || (deltaX < 0 && c.walls[3])) {
                    return true; // Blocked by wall
                }
            }
            currentX += stepX;
        }
        return false; // Reached end without hitting wall
    }

    // General case: Diagonal movement - need to check both horizontal and vertical walls
    // Calculate slope
    float slope = deltaY / deltaX;
    float inv_slope = deltaX / deltaY;

    // Iterate over cells crossed by the line segment
    while (currentX != endX || currentY != endY) {
        if (currentX >= 0 && currentX < mazeGen.width && currentY >= 0 && currentY < mazeGen.height) {
            const Cell& c = mazeGen.maze[currentY][currentX];

            // Determine next cell border to cross (horizontal or vertical)
            float t_max_x = (stepX > 0) ? ((currentX + 1) * cellSize - fromX) / deltaX :
                (currentX * cellSize - fromX) / deltaX;
            float t_max_y = (stepY > 0) ? ((currentY + 1) * cellSize - fromY) / deltaY :
                (currentY * cellSize - fromY) / deltaY;

            if (t_max_x < t_max_y) {
                // Cross vertical edge first
                if ((stepX > 0 && c.walls[1]) || (stepX < 0 && c.walls[3])) {
                    return true; // Blocked by vertical wall
                }
                currentX += stepX;
            }
            else {
                // Cross horizontal edge first
                if ((stepY > 0 && c.walls[2]) || (stepY < 0 && c.walls[0])) {
                    return true; // Blocked by horizontal wall
                }
                currentY += stepY;
            }
        }
        else {
            // Stepped out of bounds, treat as wall
            return true;
        }
    }
    return false; // Reached end without hitting wall
}

// --- New: Pathfinding Function ---
bool Monster::FindPath(const MazeGenerator& mazeGen, float cellSize, const glm::vec2& targetPos) {
    // Clear previous path
    path.clear();

    // Convert monster and player positions to grid coordinates
    int startX = static_cast<int>(floor(position.x / cellSize));
    int startY = static_cast<int>(floor(position.y / cellSize));
    int endX = static_cast<int>(floor(targetPos.x / cellSize));
    int endY = static_cast<int>(floor(targetPos.y / cellSize));

    // Boundary checks
    if (startX < 0 || startX >= mazeGen.width || startY < 0 || startY >= mazeGen.height ||
        endX < 0 || endX >= mazeGen.width || endY < 0 || endY >= mazeGen.height) {
        return false; // Start or end outside maze
    }

    // Handle case where monster is already at the target grid cell
    if (startX == endX && startY == endY) {
        // Add the target position as the single node in the path
        path.push_back(targetPos);
        return true;
    }

    // --- A* Algorithm Setup ---
    const int MAX_NODES = mazeGen.width * mazeGen.height;
    // Dynamically allocate or use fixed-size arrays on stack (be careful with large mazes)
    // Using vectors for clarity and safety, assuming they are efficient enough
    std::vector<Node> nodes(MAX_NODES);
    std::vector<bool> closedSet(MAX_NODES, false); // Visited nodes
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet; // Min-heap based on f-cost

    // Helper lambda to get node index from coordinates
    auto getNodeIndex = [&mazeGen](int x, int y) -> int {
        // Better approach: pass width or use different structure
        // Let's redefine this properly:
        return y * mazeGen.width + x; // Correct way using actual maze width
        };
  

    // Helper lambda to get coordinates from node index (if needed)
    // Not strictly needed if we keep track via pointers/nodes themselves

    // Initialize start node
    Node startNode;
    startNode.x = startX;
    startNode.y = startY;
    startNode.g = 0.0f;
    startNode.h = Heuristic(startX, startY, endX, endY);
    startNode.f = startNode.g + startNode.h;
    startNode.parent = nullptr;
    nodes[getNodeIndex(startX, startY)] = startNode;
    openSet.push(startNode);

    bool pathFound = false;
    Node* finalNode = nullptr;

    // --- A* Main Loop ---
    while (!openSet.empty()) {
        Node current = openSet.top();
        openSet.pop();

        int currentIndex = getNodeIndex(current.x, current.y);

        // Skip if already processed
        if (closedSet[currentIndex]) continue;

        closedSet[currentIndex] = true;

        // Goal reached
        if (current.x == endX && current.y == endY) {
            pathFound = true;
            finalNode = &nodes[currentIndex]; // Store pointer to goal node
            break;
        }

        // Explore neighbors (up, down, left, right)
        int dx[4] = { 0, 0, -1, 1 };
        int dy[4] = { -1, 1, 0, 0 };

        for (int i = 0; i < 4; ++i) {
            int neighborX = current.x + dx[i];
            int neighborY = current.y + dy[i];

            // Check bounds
            if (neighborX >= 0 && neighborX < mazeGen.width && neighborY >= 0 && neighborY < mazeGen.height) {
                int neighborIndex = getNodeIndex(neighborX, neighborY);

                // Skip if already visited
                if (closedSet[neighborIndex]) continue;

                // Check for wall between current and neighbor cells
                // We check the wall of the current cell leading to the neighbor
                bool wallBetween = false;
                if (dx[i] == -1) wallBetween = mazeGen.maze[current.y][current.x].walls[3]; // Left
                else if (dx[i] == 1) wallBetween = mazeGen.maze[current.y][current.x].walls[1]; // Right
                else if (dy[i] == -1) wallBetween = mazeGen.maze[current.y][current.x].walls[0]; // Up
                else if (dy[i] == 1) wallBetween = mazeGen.maze[current.y][current.x].walls[2]; // Down

                if (wallBetween) continue; // Wall blocks movement

                // Calculate tentative g score
                float tentativeG = current.g + 1.0f; // Cost to move one cell is 1

                // If this path to neighbor is better than any previous one
                if (tentativeG < nodes[neighborIndex].g) { // Initially g is likely 0 or inf
                    Node neighborNode;
                    neighborNode.x = neighborX;
                    neighborNode.y = neighborY;
                    neighborNode.g = tentativeG;
                    neighborNode.h = Heuristic(neighborX, neighborY, endX, endY);
                    neighborNode.f = neighborNode.g + neighborNode.h;
                    neighborNode.parent = &nodes[currentIndex]; // Link back to parent
                    nodes[neighborIndex] = neighborNode; // Update node data
                    openSet.push(neighborNode); // Add to open set
                }
            }
        }
    }

    // --- Reconstruct Path ---
    if (pathFound && finalNode) {
        std::vector<glm::vec2> tempPath;
        Node* currentNode = finalNode;
        while (currentNode != nullptr) {
            // Convert grid coordinates back to world coordinates (center of cell)
            float worldX = (currentNode->x + 0.5f) * cellSize;
            float worldY = (currentNode->y + 0.5f) * cellSize;
            tempPath.push_back(glm::vec2(worldX, worldY));
            currentNode = currentNode->parent;
        }

        // Reverse the path to get it from start to goal
        path.reserve(tempPath.size());
        for (auto it = tempPath.rbegin(); it != tempPath.rend(); ++it) {
            path.push_back(*it);
        }

        // Optional: Smooth path or adjust end point to be exactly player position
        // For now, last node is center of player's cell. You might want to make it the exact player pos.
        if (!path.empty()) {
            path.back() = targetPos; // Replace last node with exact target position
        }

        return true;
    }

    return false; // No path found
}