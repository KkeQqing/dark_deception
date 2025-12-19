// src/utils.h
#ifndef UTILS_H
#define UTILS_H

#include <random>

extern std::mt19937 gen; // 全局随机数生成器

inline float randFloat(float min, float max) {
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

inline int randInt(int min, int max) {
    std::uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

#endif