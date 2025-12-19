// src/UIManager.h
#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <glm/glm.hpp>
#include <string>

class UIManager {
public:
    int screenWidth, screenHeight;
    int itemsRemaining;
    float playerCooldownE; // Copy from Player
    float playerCooldownQ; // Copy from Player
    float playerCooldownEDuration;
    float playerCooldownQDuration;

    UIManager(int sw, int sh);
    void update(int remaining, float cdE, float cdQ, float cdEDur, float cdQDur);
    void draw(class Renderer& renderer);
    bool isRestartButtonClicked(double mouseX, double mouseY);
};

#endif