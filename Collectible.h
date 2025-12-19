// src/Collectible.h
#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include <glm/glm.hpp>

class Collectible {
public:
    glm::vec2 position;
    float size = 10.0f;
    bool collected = false;

    Collectible(float x, float y);
    void draw(class Renderer& renderer);
    bool checkCollection(const class Player& player);
    void reset(float x, float y);
};

#endif