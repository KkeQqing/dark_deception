// src/MazeGenerator.cpp
#include "MazeGenerator.h"

MazeGenerator::MazeGenerator(int w, int h) : width(w), height(h) {
    reset();
}

void MazeGenerator::reset() {
    maze.clear();
    maze.resize(height, std::vector<Cell>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            maze[y][x].visited = false;
            for (int i = 0; i < 4; ++i) {
                maze[y][x].walls[i] = true;
            }
        }
    }
}

void MazeGenerator::generate() {
    reset();
    carvePassagesFrom(0, 0); // Start from top-left
}

void MazeGenerator::carvePassagesFrom(int cx, int cy) {
    maze[cy][cx].visited = true;

    std::vector<std::pair<int, int>> directions = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} }; // N, E, S, W

    // Shuffle directions
    for (int i = directions.size() - 1; i > 0; --i) {
        int j = randInt(0, i);
        std::swap(directions[i], directions[j]);
    }

    for (const auto& dir : directions) {
        int nx = cx + dir.first;
        int ny = cy + dir.second;

        if (nx >= 0 && nx < width && ny >= 0 && ny < height && !maze[ny][nx].visited) {
            // Knock down the wall between (cx,cy) and (nx,ny)
            if (dir.first == 1) { // Moving East
                maze[cy][cx].walls[1] = false; // Remove Right wall of current
                maze[ny][nx].walls[3] = false; // Remove Left wall of neighbor
            }
            else if (dir.first == -1) { // Moving West
                maze[cy][cx].walls[3] = false; // Remove Left wall of current
                maze[ny][nx].walls[1] = false; // Remove Right wall of neighbor
            }
            else if (dir.second == 1) { // Moving South
                maze[cy][cx].walls[2] = false; // Remove Bottom wall of current
                maze[ny][nx].walls[0] = false; // Remove Top wall of neighbor
            }
            else if (dir.second == -1) { // Moving North
                maze[cy][cx].walls[0] = false; // Remove Top wall of current
                maze[ny][nx].walls[2] = false; // Remove Bottom wall of neighbor
            }

            carvePassagesFrom(nx, ny);
        }
    }
}