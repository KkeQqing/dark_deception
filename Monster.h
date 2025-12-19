// src/Monster.h
#ifndef MONSTER_H
#define MONSTER_H

#include <glm/glm.hpp>
#include <cmath>

class Monster {
public:
    glm::vec2 position;
    glm::vec2 targetPosition;
    float radius = 6.0f;
    float baseSpeed = 80.0f;
    float chaseSpeed = 150.0f;
    float currentSpeed = baseSpeed;
    float detectionRange = 200.0f;
    float patrolRadius = 50.0f;
    bool detectedPlayer = false;
    bool frozen = false;
    bool visible = true;
    float freezeDuration = 2.0f;
    float freezeTimer = 0.0f;

    Monster(float x, float y);
    void update(float deltaTime, const class Player& player, const class MazeGenerator& maze);
    void draw(class Renderer& renderer);
    bool detectPlayer(const Player& player, const MazeGenerator& maze);
    void freeze();
    void unfreeze();

private:
    bool isWallBetween(glm::vec2 start, glm::vec2 end, const MazeGenerator& maze) const;
    void moveToTarget(float deltaTime);
};

#endif