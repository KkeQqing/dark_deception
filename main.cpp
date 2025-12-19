// src/main.cpp
#include "Game.h"
#include <iostream>
#include <cstdlib> // For srand, rand
#include <ctime>   // For time

int main() {
	// 随机数种子
    srand(static_cast<unsigned int>(time(nullptr)));

    Game game;
    if (!game.initialize()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return -1;
    }

    game.run();

    return 0;
}