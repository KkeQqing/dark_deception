// src/MazeGenerator.h
#ifndef MAZEGENERATOR_H
#define MAZEGENERATOR_H

#include <vector>
#include <stack>
#include <utility> // for std::pair
#include "utils.h"

struct Cell {
    bool visited = false;
    // Walls: Top, Right, Bottom, Left
    bool walls[4] = { true, true, true, true };
};

class MazeGenerator {
public:
    int width, height;
    std::vector<std::vector<Cell>> maze;

    MazeGenerator(int w, int h);
    void generate();
    void reset();

private:
    void carvePassagesFrom(int cx, int cy);
    std::vector<std::pair<int, int>> getUnvisitedNeighbors(int x, int y);
};

#endif