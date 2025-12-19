// src/UIManager.cpp
#include "UIManager.h"
#include "Renderer.h"
#include <sstream>
#include <iomanip> // for std::setprecision
#include <cmath> // for round

UIManager::UIManager(int sw, int sh) : screenWidth(sw), screenHeight(sh),
itemsRemaining(0), playerCooldownE(0.0f), playerCooldownQ(0.0f),
playerCooldownEDuration(0.0f), playerCooldownQDuration(0.0f) {
}

void UIManager::update(int remaining, float cdE, float cdQ, float cdEDur, float cdQDur) {
    itemsRemaining = remaining;
    playerCooldownE = cdE;
    playerCooldownQ = cdQ;
    playerCooldownEDuration = cdEDur;
    playerCooldownQDuration = cdQDur;
}

void UIManager::draw(Renderer& renderer) {
    // Draw background bar for UI
    renderer.drawRect(glm::vec2(0, screenHeight - 40), glm::vec2(screenWidth, 40), glm::vec3(0.2f, 0.2f, 0.2f));

    // Draw item count
    std::ostringstream oss;
    oss << itemsRemaining;
    renderer.drawText(oss.str(), glm::vec2(screenWidth / 2.0f - 20, screenHeight - 30), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

    // Draw Skill Icons Placeholder Text (Replace with actual icons/sprites later)
    // E Skill (Left)
    float iconSize = 20.0f;
    glm::vec2 eIconPos(20, screenHeight - 30);
    renderer.drawText("E", eIconPos, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    // Cooldown overlay for E
    if (playerCooldownE > 0.0f && playerCooldownEDuration > 0.0f) {
        float alpha = playerCooldownE / playerCooldownEDuration;
        renderer.drawRect(eIconPos - glm::vec2(iconSize / 2, iconSize / 2), glm::vec2(iconSize, iconSize * alpha), glm::vec3(0.5f, 0.5f, 0.5f));
    }

    // Q Skill (Right)
    glm::vec2 qIconPos(screenWidth - 40, screenHeight - 30);
    renderer.drawText("Q", qIconPos, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    // Cooldown overlay for Q
    if (playerCooldownQ > 0.0f && playerCooldownQDuration > 0.0f) {
        float alpha = playerCooldownQ / playerCooldownQDuration;
        renderer.drawRect(qIconPos - glm::vec2(iconSize / 2, iconSize / 2), glm::vec2(iconSize, iconSize * alpha), glm::vec3(0.5f, 0.5f, 0.5f));
    }

    // Restart Button Placeholder
    renderer.drawText("R", glm::vec2(10, 10), 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
}

bool UIManager::isRestartButtonClicked(double mouseX, double mouseY) {
    // Simple box check for restart button area
    return (mouseX >= 0 && mouseX <= 30 && mouseY >= 0 && mouseY <= 30);
}