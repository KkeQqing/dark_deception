// src/Player.h
#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include "InputHandler.h"
#include "AudioSystem.h"
#include <cmath>

class Game; // Forward declaration

class Player {
public:
    glm::vec2 position;
    float radius = 8.0f;
    float speed = 150.0f;
    float baseSpeed = 150.0f;
    float acceleratedSpeed = 300.0f;

    bool isAccelerating = false;
    float accelDuration = 3.0f;
    float accelTimer = 0.0f;
    float cooldownE = 0.0f;
    float cooldownEDuration = 15.0f;

    float cooldownQ = 0.0f;
    float cooldownQDuration = 20.0f;

    Player(float x, float y);
    void update(float deltaTime, const class MazeGenerator& maze, AudioSystem& audio);
    void draw(class Renderer& renderer);
    bool checkCollision(const glm::vec2& otherPos, float otherRadius) const;
    void reset(float x, float y);
};

#endif