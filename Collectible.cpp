// src/Collectible.cpp
#include "Collectible.h"
#include "Player.h"
#include "Renderer.h"

Collectible::Collectible(float x, float y) : position(x, y), collected(false) {}

void Collectible::draw(Renderer& renderer) {
    if (!collected) {
        renderer.drawSquare(position, size, glm::vec3(1.0f, 0.0f, 1.0f)); // Purple
    }
}

bool Collectible::checkCollection(const Player& player) {
    if (!collected && player.checkCollision(position, size / 2.0f)) {
        collected = true;
        return true;
    }
    return false;
}

void Collectible::reset(float x, float y) {
    position = glm::vec2(x, y);
    collected = false;
}