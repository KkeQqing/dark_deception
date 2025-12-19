// src/Monster.cpp
#include "Monster.h"
#include "Player.h"
#include "MazeGenerator.h"
#include "Renderer.h"
#include <cstdlib> // For rand
#include <ctime>   // For time
#include <glm/gtx/intersect.hpp> // Optional for more precise line intersection

#define M_PI 3.14159265358979323846

Monster::Monster(float x, float y) : position(x, y), targetPosition(x, y) {}

void Monster::update(float deltaTime, const Player& player, const MazeGenerator& maze) {
    if (frozen) {
        freezeTimer -= deltaTime;
        if (freezeTimer <= 0.0f) {
            unfreeze();
        }
        return; // Don't move or detect when frozen
    }

    detectedPlayer = detectPlayer(player, maze);

    if (detectedPlayer) {
        currentSpeed = chaseSpeed;
        targetPosition = player.position;
    }
    else {
        currentSpeed = baseSpeed;
        // Simple patrol: occasionally pick a new random target nearby
        if ((rand() % 100) < 2) { // Roughly every 50 frames at 60fps
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
            float distance = static_cast<float>(rand()) / RAND_MAX * patrolRadius;
            targetPosition = position + glm::vec2(cos(angle) * distance, sin(angle) * distance);
        }
    }

    moveToTarget(deltaTime);

    // Visibility based on distance to player
    float distanceToPlayer = glm::distance(position, player.position);
    visible = distanceToPlayer < 150.0f; // Adjust visibility range as needed
}

void Monster::draw(Renderer& renderer) {
    if (visible) {
        glm::vec3 color = frozen ? glm::vec3(0.5f, 0.5f, 1.0f) : glm::vec3(0.0f, 0.0f, 1.0f); // Blue, lighter when frozen
        renderer.drawTriangle(position, radius, color);
    }
}

bool Monster::detectPlayer(const Player& player, const MazeGenerator& maze) {
    glm::vec2 direction = player.position - position;
    float distance = glm::length(direction);

    if (distance > detectionRange) {
        return false;
    }

    // Simplified raycasting: check direct line of sight
    // A more robust method would involve checking against actual wall segments
    return !isWallBetween(position, player.position, maze);
}

bool Monster::isWallBetween(glm::vec2 start, glm::vec2 end, const MazeGenerator& maze) const {
    // This is a simplified version. It checks cells along the line.
    // A better approach uses Bresenham's line algorithm or DDA.

    int x0 = static_cast<int>(start.x);
    int y0 = static_cast<int>(start.y);
    int x1 = static_cast<int>(end.x);
    int y1 = static_cast<int>(end.y);

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int x = x0;
    int y = y0;

    while (true) {
        // Check bounds
        if (x < 0 || x >= maze.width || y < 0 || y >= maze.height) {
            // Outside maze counts as blocked
            return true;
        }

        // Check for wall hit
        glm::vec2 cellCenter(x + 0.5f, y + 0.5f);
        glm::vec2 toStart = start - cellCenter;
        glm::vec2 toEnd = end - cellCenter;

        // If the line segment crosses the center significantly, check walls
        // This is still approximate but simpler than full line-box intersection
        if (abs(toStart.x) < 0.6f && abs(toStart.y) < 0.6f) {
            // We are inside the cell, check if the line enters/leaves through a wall
            glm::vec2 lineDir = glm::normalize(end - start);
            // Determine entry/exit side based on direction and cell edges
            // This is quite complex accurately, so we simplify:
            // Just check if any wall exists that could block the direct path roughly.
            // A very rough approximation:
            if (x == x0 && y == y0) {
                // Starting cell, skip wall check to avoid self-block
            }
            else if (x == x1 && y == y1) {
                // Ending cell, skip wall check
            }
            else {
                // Check if the line likely crosses a wall in this cell.
                // This is still not perfect but avoids full intersection math.
                // For simplicity, let's just check if the cell has all walls (dead end)
                // or if the movement direction suggests hitting a wall.
                // This part needs refinement for truly accurate raycasting.
                // Let's assume if the cell is visited in the path, it might have a wall blocking.
                // A quick hack: if the cell is not the start or end, and the line goes roughly horizontal/vertical,
                // check relevant walls.
                if (abs(lineDir.x) > abs(lineDir.y)) {
                    // More horizontal
                    if (lineDir.x > 0 && maze.maze[y][x].walls[1]) return true; // Moving right, hit right wall
                    if (lineDir.x < 0 && maze.maze[y][x].walls[3]) return true; // Moving left, hit left wall
                }
                else {
                    // More vertical
                    if (lineDir.y > 0 && maze.maze[y][x].walls[2]) return true; // Moving down, hit bottom wall
                    if (lineDir.y < 0 && maze.maze[y][x].walls[0]) return true; // Moving up, hit top wall
                }
            }
        }


        if (x == x1 && y == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    return false; // No wall found blocking the path
}


void Monster::moveToTarget(float deltaTime) {
    glm::vec2 direction = targetPosition - position;
    float distance = glm::length(direction);

    if (distance > 1.0f) { // Stop threshold
        direction = glm::normalize(direction);
        position += direction * currentSpeed * deltaTime;
    }
}

void Monster::freeze() {
    frozen = true;
    freezeTimer = freezeDuration;
    currentSpeed = 0.0f; // Stop moving
}

void Monster::unfreeze() {
    frozen = false;
    currentSpeed = baseSpeed;
}