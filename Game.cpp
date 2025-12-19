// src/Game.cpp
#include "Game.h"
#include "InputHandler.h"
#include <iostream>
#include <algorithm> // for std::count_if


Game::Game() : window(nullptr), screenWidth(800), screenHeight(600), cellSize(20.0f),
gameOver(false), victory(false), alertTriggered(false), lastFrameTime(0.0f) {
} // 生成器初始化列表

Game::~Game() {
    if (window) {
        glfwDestroyWindow(window);
    }
	glfwTerminate(); // glfw终止
}

bool Game::initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // 配置 GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建窗口
    window = glfwCreateWindow(screenWidth, screenHeight, "Dark Deception 2D", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

	// 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

	// 设置视口 
    glfwSetKeyCallback(window, InputHandler::keyCallback);
    glfwSetCursorPosCallback(window, InputHandler::cursorPosCallback);
    glfwSetMouseButtonCallback(window, InputHandler::mouseButtonCallback);

	// 启用混合以支持透明度
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 初始化子系统
    renderer = std::make_unique<Renderer>(screenWidth, screenHeight);
    uiManager = std::make_unique<UIManager>(screenWidth, screenHeight);
    audio = std::make_unique<AudioSystem>();

	// 加载音频资源
    audio->loadSound("collect", "assets/sounds/collect.wav");
    audio->loadSound("skill_e", "assets/sounds/skill_e.wav");
    audio->loadSound("skill_q", "assets/sounds/skill_q.wav");
    audio->loadSound("alert", "assets/sounds/alert.wav");
	audio->loadMusic("assets/sounds/bg_music.mp3"); // 背景音乐
    audio->playMusic();

	// 初始化游戏状态
    resetGame();

    return true;
}

// 主游戏循环
void Game::run() {
    lastFrameTime = static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrameTime;
        lastFrameTime = currentFrame;

        processInput(deltaTime);
        update(deltaTime);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
        InputHandler::update();
    }
}

// 处理输入
void Game::processInput(float deltaTime) {
    if (InputHandler::isKeyPressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }

    if (gameOver || victory) {
        if (InputHandler::isKeyPressed(GLFW_KEY_R) || InputHandler::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (uiManager->isRestartButtonClicked(InputHandler::mouseX, InputHandler::mouseY) ||
                InputHandler::isKeyPressed(GLFW_KEY_R)) {
                resetGame();
            }
        }
    }
}

// 更新游戏状态
void Game::update(float deltaTime) {
    if (gameOver || victory) return;

    player->update(deltaTime, *maze, *audio);

    // Handle Q skill effect on monsters
    if (player->cooldownQ > player->cooldownQDuration - 0.1f) { // Just activated
        for (auto& monster : monsters) {
            monster.freeze();
        }
    }

    for (auto& monster : monsters) {
        monster.update(deltaTime, *player, *maze);
        if (monster.detectedPlayer && !alertTriggered) {
            alertTriggered = true;
            audio->playSound("alert");
        }
    }

    checkCollisions();
    checkWinCondition();

    // Update UI data
    int remainingItems = std::count_if(collectibles.begin(), collectibles.end(),
        [](const Collectible& c) { return !c.collected; });
    uiManager->update(remainingItems, player->cooldownE, player->cooldownQ,
        player->cooldownEDuration, player->cooldownQDuration);
}

// 渲染游戏
void Game::render() {
    renderer->beginScene();

    renderer->drawMaze(*maze);
    for (auto& collectible : collectibles) {
        collectible.draw(*renderer);
    }
    for (auto& monster : monsters) {
        monster.draw(*renderer);
    }
    player->draw(*renderer);

    renderer->drawAlertFlash(alertTriggered);
    alertTriggered = false; // Reset after drawing

    uiManager->draw(*renderer);

    if (gameOver) {
        renderer->drawText("GAME OVER!", glm::vec2(screenWidth / 2 - 50, screenHeight / 2), 1.0f, glm::vec3(1, 0, 0));
    }
    else if (victory) {
        renderer->drawText("YOU WIN!", glm::vec2(screenWidth / 2 - 50, screenHeight / 2), 1.0f, glm::vec3(0, 1, 0));
    }

    renderer->endScene();
}

// 重置游戏状态
void Game::resetGame() {
    gameOver = false;
    victory = false;
    alertTriggered = false;

    maze = std::make_unique<MazeGenerator>(20, 20); // 20x20 cells
    maze->generate();

    placeEntities();

    // Reset player
    if (player) {
        player->reset(0.5f, 0.5f); // Start slightly offset in the first cell
    }
    else {
        player = std::make_unique<Player>(0.5f, 0.5f);
    }

    // Reset UI
    totalItems = collectibles.size();
    int remainingItems = std::count_if(collectibles.begin(), collectibles.end(),
        [](const Collectible& c) { return !c.collected; });
    uiManager->update(remainingItems, player->cooldownE, player->cooldownQ,
        player->cooldownEDuration, player->cooldownQDuration);
}

// 放置玩家、怪物和收集品
void Game::placeEntities() {
    monsters.clear();
    collectibles.clear();

    // Place monsters - avoid placing directly on player start
    for (int i = 0; i < 3; ++i) {
        int mx, my;
        do {
            mx = randInt(10, maze->width - 1); // Avoid corners/start area
            my = randInt(10, maze->height - 1);
        } while (mx == 0 && my == 0); // Simple check, could be more robust

        monsters.emplace_back((mx + 0.5f) * cellSize, (my + 0.5f) * cellSize);
    }

    // Place collectibles - scatter them around
    totalItems = 10; // Or calculate based on maze size
    for (int i = 0; i < totalItems; ++i) {
        int cx = randInt(1, maze->width - 2);
        int cy = randInt(1, maze->height - 2);
        collectibles.emplace_back((cx + 0.5f) * cellSize, (cy + 0.5f) * cellSize);
    }
}

// 检查碰撞
void Game::checkCollisions() {
    for (auto& collectible : collectibles) {
        if (collectible.checkCollection(*player)) {
            audio->playSound("collect");
        }
    }

    for (const auto& monster : monsters) {
        if (monster.visible && player->checkCollision(monster.position, monster.radius)) {
            gameOver = true;
            return; // Stop checking after game over
        }
    }
}

// 检查胜利条件
void Game::checkWinCondition() {
    int remainingItems = std::count_if(collectibles.begin(), collectibles.end(),
        [](const Collectible& c) { return !c.collected; });

    if (remainingItems == 0) {
        victory = true;
    }
}