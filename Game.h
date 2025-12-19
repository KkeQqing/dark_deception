// src/Game.h
#ifndef GAME_H  
#define GAME_H

#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include "MazeGenerator.h"
#include "Player.h"
#include "Monster.h"
#include "Collectible.h"
#include "Renderer.h"
#include "UIManager.h"
#include "AudioSystem.h"

class Game {
public:
    GLFWwindow* window;
    int screenWidth, screenHeight;
	float cellSize; // 每个迷宫单元格大小

	std::unique_ptr<MazeGenerator> maze; // 迷宫生成器
	std::unique_ptr<Player> player; // 玩家
	std::vector<Monster> monsters; // 怪物列表
	std::vector<Collectible> collectibles; // 收集品列表
	std::unique_ptr<Renderer> renderer; // 渲染器
	std::unique_ptr<UIManager> uiManager; // UI管理器
	std::unique_ptr<AudioSystem> audio; // 音频系统

	int totalItems; // 总收集品数量
	bool gameOver; // 游戏结束标志
	bool victory; // 胜利标志
	bool alertTriggered; // 警报触发标志
	float lastFrameTime; // 上一帧时间戳

    Game();
    ~Game();

	bool initialize(); // 初始化游戏
	void run(); // 主游戏循环
	void processInput(float deltaTime); // 处理输入
	void update(float deltaTime); // 更新游戏状态
	void render(); // 渲染游戏
	void resetGame(); // 重置游戏状态

private:
	void placeEntities(); // 放置玩家、怪物和收集品
	void checkCollisions(); // 检查碰撞
	void checkWinCondition(); // 检查胜利条件
};

#endif