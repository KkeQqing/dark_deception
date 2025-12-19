// src/Renderer.h
#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include "Shader.h"

class Renderer {
public:
    Shader shader;
    unsigned int VAO, VBO; // For dynamic geometry
    unsigned int textVAO, textVBO; // For text rendering (placeholder)
    glm::mat4 projection;

    Renderer(int screenWidth, int screenHeight);
    ~Renderer();

    void beginScene();
    void endScene();

    void drawCircle(const glm::vec2& center, float radius, const glm::vec3& color);
    void drawSquare(const glm::vec2& center, float size, const glm::vec3& color);
    void drawTriangle(const glm::vec2& center, float radius, const glm::vec3& color);
    void drawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec3& color); // For UI elements

    // Placeholder for text drawing - needs proper font loading
    void drawText(const std::string& text, const glm::vec2& position, float scale, const glm::vec3& color);

    void drawMaze(const class MazeGenerator& maze);

    void drawAlertFlash(bool trigger); // For red flash effect

private:
    void setupBuffers();
    void setupTextBuffers(); // Placeholder
};

#endif