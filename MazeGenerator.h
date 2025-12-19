#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <stack>

// 迷宫单元格结构体
struct Cell {
    bool visited = false;
    bool walls[4] = { true, true, true, true }; // 上右下左
};

// 迷宫生成类
class MazeGenerator {
public:
    int width, height;
    std::vector<std::vector<Cell>> maze;
    std::mt19937 rng;

    MazeGenerator(int w, int h) : width(w), height(h), rng(std::random_device{}()) {
        maze.resize(height, std::vector<Cell>(width));
    }

    void Generate() {
        // 重置迷宫
        for (auto& row : maze) {
            for (auto& cell : row) {
                cell.visited = false;
                for (int i = 0; i < 4; ++i) cell.walls[i] = true;
            }
        }
        generateRecursiveBacktracker(0, 0);
    }

private:
	// 递归回溯算法生成迷宫
    void generateRecursiveBacktracker(int x, int y) {
        static std::vector<std::pair<int, int>> directions = { {0,-1}, {1,0}, {0,1}, {-1,0} };
        maze[y][x].visited = true;
        std::shuffle(directions.begin(), directions.end(), rng);

        for (const auto& dir : directions) {
            int nx = x + dir.first;
            int ny = y + dir.second;

            if (nx >= 0 && nx < width && ny >= 0 && ny < height && !maze[ny][nx].visited) {
                int wall_to_remove_current = 0, wall_to_remove_next = 2;
                if (dir.first == 1) { wall_to_remove_current = 1; wall_to_remove_next = 3; }
                else if (dir.first == -1) { wall_to_remove_current = 3; wall_to_remove_next = 1; }
                else if (dir.second == 1) { wall_to_remove_current = 2; wall_to_remove_next = 0; }
                else if (dir.second == -1) { wall_to_remove_current = 0; wall_to_remove_next = 2; }

                maze[y][x].walls[wall_to_remove_current] = false;
                maze[ny][nx].walls[wall_to_remove_next] = false;

                generateRecursiveBacktracker(nx, ny);
            }
        }
    }
};