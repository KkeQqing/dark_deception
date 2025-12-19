// src/Player.cpp
#include "Player.h"
#include "MazeGenerator.h"
#include "Renderer.h"

Player::Player(float x, float y) : position(x, y) {}

void Player::update(float deltaTime, const MazeGenerator& maze, AudioSystem& audio) {
    // Update cooldowns
    if (cooldownE > 0.0f) cooldownE -= deltaTime;
    if (cooldownQ > 0.0f) cooldownQ -= deltaTime;
    if (accelTimer > 0.0f) {
        accelTimer -= deltaTime;
        if (accelTimer <= 0.0f) {
            isAccelerating = false;
            speed = baseSpeed;
        }
    }

    // Handle input
    glm::vec2 moveDirection(0.0f, 0.0f);
    if (InputHandler::isKeyPressed(GLFW_KEY_W)) moveDirection.y -= 1.0f;
    if (InputHandler::isKeyPressed(GLFW_KEY_S)) moveDirection.y += 1.0f;
    if (InputHandler::isKeyPressed(GLFW_KEY_A)) moveDirection.x -= 1.0f;
    if (InputHandler::isKeyPressed(GLFW_KEY_D)) moveDirection.x += 1.0f;

    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
    }

    glm::vec2 newPosition = position + moveDirection * speed * deltaTime;

    // Simple collision with maze boundaries (assuming grid size is 1 unit per cell)
    int cellX = static_cast<int>(newPosition.x);
    int cellY = static_cast<int>(newPosition.y);
    if (cellX >= 0 && cellX < maze.width && cellY >= 0 && cellY < maze.height) {
        // Check walls based on movement direction
        bool canMoveX = true;
        bool canMoveY = true;

        if (moveDirection.x > 0) { // Moving right
            if (maze.maze[cellY][cellX].walls[1]) canMoveX = false;
        }
        else if (moveDirection.x < 0) { // Moving left
            if (cellX > 0 && maze.maze[cellY][cellX - 1].walls[1]) canMoveX = false;
        }

        if (moveDirection.y > 0) { // Moving down
            if (maze.maze[cellY][cellX].walls[2]) canMoveY = false;
        }
        else if (moveDirection.y < 0) { // Moving up
            if (cellY > 0 && maze.maze[cellY - 1][cellX].walls[2]) canMoveY = false;
        }

        if (canMoveX) position.x = newPosition.x;
        if (canMoveY) position.y = newPosition.y;
    }


    // Handle skills
    if (InputHandler::isKeyPressed(GLFW_KEY_E) && cooldownE <= 0.0f) {
        isAccelerating = true;
        accelTimer = accelDuration;
        speed = acceleratedSpeed;
        cooldownE = cooldownEDuration;
        audio.playSound("skill_e");
    }

    if (InputHandler::isKeyPressed(GLFW_KEY_Q) && cooldownQ <= 0.0f) {
        // Signal to Game to freeze monsters
        cooldownQ = cooldownQDuration;
        audio.playSound("skill_q");
    }
}

void Player::draw(Renderer& renderer) {
    renderer.drawCircle(position, radius, glm::vec3(1.0f, 0.0f, 0.0f)); // Red
}

bool Player::checkCollision(const glm::vec2& otherPos, float otherRadius) const {
    float distance = glm::distance(position, otherPos);
    return distance < (radius + otherRadius);
}

void Player::reset(float x, float y) {
    position = glm::vec2(x, y);
    speed = baseSpeed;
    isAccelerating = false;
    accelTimer = 0.0f;
    cooldownE = 0.0f;
    cooldownQ = 0.0f;
}