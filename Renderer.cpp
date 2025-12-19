// src/Renderer.cpp
#include "Renderer.h"
#include "MazeGenerator.h"
#include <iostream>
#include <cmath> // M_PI

#define M_PI 3.14159265358979323846

Renderer::Renderer(int screenWidth, int screenHeight) : shader("assets/shaders/vertex_shader.vs", "assets/shaders/fragment_shader.fs") {
    projection = glm::ortho(0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, -1.0f, 1.0f);
    setupBuffers();
    setupTextBuffers(); // Initialize text buffers
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
}

void Renderer::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::setupTextBuffers() {
    // Placeholder initialization
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void Renderer::beginScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glClear(GL_COLOR_BUFFER_BIT);
    shader.use();
    shader.setMat4("projection", projection);
}

void Renderer::endScene() {
    // Nothing specific needed here for basic rendering
}

void Renderer::drawCircle(const glm::vec2& center, float radius, const glm::vec3& color) {
    const int segments = 32;
    std::vector<float> vertices;
    vertices.reserve((segments + 2) * 6); // Center + segments + closing point

    // Center vertex
    vertices.insert(vertices.end(), { center.x, center.y, 0.0f, color.r, color.g, color.b });

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        vertices.insert(vertices.end(), { x, y, 0.0f, color.r, color.g, color.b });
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::drawSquare(const glm::vec2& center, float size, const glm::vec3& color) {
    float halfSize = size / 2.0f;
    float vertices[] = {
        // Positions          // Colors
        center.x - halfSize, center.y - halfSize, 0.0f,  color.r, color.g, color.b, // Bottom Left
        center.x + halfSize, center.y - halfSize, 0.0f,  color.r, color.g, color.b, // Bottom Right
        center.x + halfSize, center.y + halfSize, 0.0f,  color.r, color.g, color.b, // Top Right
        center.x - halfSize, center.y + halfSize, 0.0f,  color.r, color.g, color.b  // Top Left
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &EBO);
}

void Renderer::drawTriangle(const glm::vec2& center, float radius, const glm::vec3& color) {
    float vertices[] = {
        // Positions          // Colors
        center.x, center.y + radius * 0.8f, 0.0f,  color.r, color.g, color.b, // Top
        center.x - radius, center.y - radius * 0.5f, 0.0f,  color.r, color.g, color.b, // Bottom Left
        center.x + radius, center.y - radius * 0.5f, 0.0f,  color.r, color.g, color.b  // Bottom Right
    };

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::drawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec3& color) {
    float vertices[] = {
        pos.x, pos.y, 0.0f, color.r, color.g, color.b,
        pos.x + size.x, pos.y, 0.0f, color.r, color.g, color.b,
        pos.x + size.x, pos.y + size.y, 0.0f, color.r, color.g, color.b,
        pos.x, pos.y + size.y, 0.0f, color.r, color.g, color.b
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &EBO);
}


void Renderer::drawText(const std::string& text, const glm::vec2& position, float scale, const glm::vec3& color) {
    // This is a placeholder implementation. Real text rendering requires loading fonts (like with stb_truetype).
    // For now, we'll just print to console or draw simple shapes representing letters.
    // A real implementation would involve:
    // 1. Loading a TTF font file.
    // 2. Generating character textures or VBOs for each glyph.
    // 3. Rendering textured quads for each character.
    // Here, we'll simulate by drawing rectangles for 'E', 'Q', 'R' and numbers.

    float charWidth = 10.0f * scale;
    float charHeight = 15.0f * scale;
    glm::vec2 currentPos = position;

    for (char c : text) {
        switch (c) {
        case 'E':
            drawRect(currentPos, glm::vec2(charWidth, charHeight), color);
            drawRect(currentPos + glm::vec2(0, charHeight * 0.33f), glm::vec2(charWidth, charHeight * 0.33f), glm::vec3(0, 0, 0)); // Cut out middle
            break;
        case 'Q':
            drawCircle(currentPos + glm::vec2(charWidth / 2, charHeight / 2), charWidth / 2, color);
            drawRect(currentPos + glm::vec2(charWidth * 0.7f, charHeight * 0.3f), glm::vec2(charWidth * 0.3f, charHeight * 0.4f), glm::vec3(0, 0, 0)); // Tail
            break;
        case 'R':
            drawRect(currentPos, glm::vec2(charWidth * 0.3f, charHeight), color); // Stem
            drawRect(currentPos + glm::vec2(charWidth * 0.3f, 0), glm::vec2(charWidth * 0.7f, charHeight * 0.5f), color); // Top
            drawRect(currentPos + glm::vec2(charWidth * 0.3f, charHeight * 0.5f), glm::vec2(charWidth * 0.7f, charHeight * 0.5f), color); // Bottom
            drawRect(currentPos + glm::vec2(charWidth * 0.7f, charHeight * 0.25f), glm::vec2(charWidth * 0.3f, charHeight * 0.25f), glm::vec3(0, 0, 0)); // Diagonal cut
            break;
        default: // Assume digit for simplicity
            drawRect(currentPos, glm::vec2(charWidth, charHeight), color);
            // Add logic to "cut out" parts of the rectangle to form digits 0-9
            // This is complex, so we'll leave it as a solid rect for now.
            break;
        }
        currentPos.x += charWidth + 2.0f; // Spacing
    }
}


void Renderer::drawMaze(const MazeGenerator& maze) {
    float cellSize = 1.0f; // Assuming 1 unit per cell in world space
    glm::vec3 wallColor(0.8f, 0.8f, 0.8f); // Light gray

    std::vector<float> wallVertices;

    for (int y = 0; y < maze.height; ++y) {
        for (int x = 0; x < maze.width; ++x) {
            const Cell& cell = maze.maze[y][x];
            float posX = x * cellSize;
            float posY = y * cellSize;

            // Top wall
            if (cell.walls[0]) {
                wallVertices.insert(wallVertices.end(), {
                    posX, posY, 0.0f, wallColor.r, wallColor.g, wallColor.b,
                    posX + cellSize, posY, 0.0f, wallColor.r, wallColor.g, wallColor.b
                    });
            }
            // Right wall
            if (cell.walls[1]) {
                wallVertices.insert(wallVertices.end(), {
                    posX + cellSize, posY, 0.0f, wallColor.r, wallColor.g, wallColor.b,
                    posX + cellSize, posY + cellSize, 0.0f, wallColor.r, wallColor.g, wallColor.b
                    });
            }
            // Bottom wall
            if (cell.walls[2]) {
                wallVertices.insert(wallVertices.end(), {
                    posX, posY + cellSize, 0.0f, wallColor.r, wallColor.g, wallColor.b,
                    posX + cellSize, posY + cellSize, 0.0f, wallColor.r, wallColor.g, wallColor.b
                    });
            }
            // Left wall
            if (cell.walls[3]) {
                wallVertices.insert(wallVertices.end(), {
                    posX, posY, 0.0f, wallColor.r, wallColor.g, wallColor.b,
                    posX, posY + cellSize, 0.0f, wallColor.r, wallColor.g, wallColor.b
                    });
            }
        }
    }

    if (!wallVertices.empty()) {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, wallVertices.size() * sizeof(float), wallVertices.data(), GL_STATIC_DRAW);

        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);

        glDrawArrays(GL_LINES, 0, wallVertices.size() / 6);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void Renderer::drawAlertFlash(bool trigger) {
    if (trigger) {
        // Draw a full-screen red quad
        float vertices[] = {
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom Left
            static_cast<float>(projection[0][0]), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom Right
            static_cast<float>(projection[0][0]), static_cast<float>(-projection[1][1]), 0.0f, 1.0f, 0.0f, 0.0f, // Top Right
            0.0f, static_cast<float>(-projection[1][1]), 0.0f, 1.0f, 0.0f, 0.0f  // Top Left
        };
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        unsigned int EBO;
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glDeleteBuffers(1, &EBO);
    }
}