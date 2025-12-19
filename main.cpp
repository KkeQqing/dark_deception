#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h> // 必须在 glfw3.h 之前
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "Shader.h"
#include "MazeGenerator.h"
#include "Player.h" // 包含 Player 头文件
#include "Monster.h" // 包含 Monster 头文件
#include "Collectible.h"
#include "Renderer.h"
#include "AudioSystem.h"

// 全局变量用于回调
bool keys[1024]; // 按键状态
bool alertTriggered = false; // 警报状态
AudioSystem audioSystem; // 全局音频系统实例

void framebuffer_size_callback(GLFWwindow* window, int width, int height); // 窗口大小回调
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode); // 键盘回调
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods); // 鼠标按钮回调

// --- 新增: 重置游戏函数 ---
void ResetGame(MazeGenerator& mazeGen, Player& player, std::vector<Monster>& monsters, std::vector<Collectible>& collectibles, int& score, const int MAZE_WIDTH, const int MAZE_HEIGHT, const float CELL_SIZE) {
    mazeGen.Generate();

    // --- 修改 Player 初始化 ---
    player = Player(0, 0, CELL_SIZE); // 从 (0,0) 单元格开始

    player.speed = 150.0f;
    player.isAccelerating = false;
    player.accelTimer = 0.0f;
    player.cooldownE = 0.0f;
    player.cooldownQ = 0.0f;

    monsters.clear();
    monsters.emplace_back(5 * CELL_SIZE + CELL_SIZE / 2, 5 * CELL_SIZE + CELL_SIZE / 2);
    monsters.emplace_back(10 * CELL_SIZE + CELL_SIZE / 2, 10 * CELL_SIZE + CELL_SIZE / 2);
    monsters.emplace_back(15 * CELL_SIZE + CELL_SIZE / 2, 15 * CELL_SIZE + CELL_SIZE / 2);

    collectibles.clear();
    for (int i = 0; i < 5; ++i) {
        int x = rand() % MAZE_WIDTH;
        int y = rand() % MAZE_HEIGHT;
        collectibles.emplace_back(x * CELL_SIZE + CELL_SIZE / 2, y * CELL_SIZE + CELL_SIZE / 2);
    }

    score = collectibles.size();
    alertTriggered = false;
}

int main()
{
    srand(static_cast<unsigned int>(time(0)));
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int SCR_WIDTH = 800;
    const int SCR_HEIGHT = 600;
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "dark deception", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 初始化音频
    audioSystem.LoadSound("collect", "D:/C++/dark_deception/assets/sounds/collect.wav");
    audioSystem.LoadSound("skill_e", "D:/C++/dark_deception/assets/sounds/skill_e.wav");
    audioSystem.LoadSound("skill_q", "D:/C++/dark_deception/assets/sounds/skill_q.wav");
    audioSystem.LoadSound("alert", "D:/C++/dark_deception/assets/sounds/alert.wav");

    // 初始化游戏对象
    const int MAZE_WIDTH = 20;
    const int MAZE_HEIGHT = 20;
    const float CELL_SIZE = 25.0f;
    MazeGenerator mazeGen(MAZE_WIDTH, MAZE_HEIGHT);
    mazeGen.Generate();

    // 简单放置玩家在起点
    Player player(0, 0, CELL_SIZE);

    // 放置怪物
    std::vector<Monster> monsters;
    monsters.emplace_back(5 * CELL_SIZE + CELL_SIZE / 2, 5 * CELL_SIZE + CELL_SIZE / 2);
    monsters.emplace_back(10 * CELL_SIZE + CELL_SIZE / 2, 10 * CELL_SIZE + CELL_SIZE / 2);
    monsters.emplace_back(15 * CELL_SIZE + CELL_SIZE / 2, 15 * CELL_SIZE + CELL_SIZE / 2);

    // 放置收集品
    std::vector<Collectible> collectibles;
    for (int i = 0; i < 5; ++i) {
        int x = rand() % MAZE_WIDTH;
        int y = rand() % MAZE_HEIGHT;
        collectibles.emplace_back(x * CELL_SIZE + CELL_SIZE / 2, y * CELL_SIZE + CELL_SIZE / 2);
    }

    Renderer renderer(SCR_WIDTH, SCR_HEIGHT);

    float lastFrame = 0.0f;
    int score = collectibles.size();

    // --- 新增: 胜利状态管理 ---
    bool gameWon = false;
    float victoryTimer = 0.0f;
    const float victoryDisplayTime = 3.0f; // 胜利消息显示时间

    // 游戏主循环
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // --- 修改输入处理和移动逻辑 ---
        // 不再直接根据 WASD 计算连续的 newPosition
        // 而是根据按键触发单元格间的跳跃式移动

        // --- 触发移动 ---
        if (keys[GLFW_KEY_W]) {
            player.TryMove(0, -1, mazeGen, CELL_SIZE); // 向上
        }
        if (keys[GLFW_KEY_S]) {
            player.TryMove(0, 1, mazeGen, CELL_SIZE); // 向下
        }
        if (keys[GLFW_KEY_A]) {
            player.TryMove(-1, 0, mazeGen, CELL_SIZE); // 向左
        }
        if (keys[GLFW_KEY_D]) {
            player.TryMove(1, 0, mazeGen, CELL_SIZE); // 向右
        }


        // --- 执行移动动画 (必须在 Update 之前或之后调用，取决于设计) ---
        // 在这里调用 PerformMovement 是合理的
        player.PerformMovement(deltaTime, CELL_SIZE); // 传入正确的 CELL_SIZE


        // --- 技能处理 ---
        if (keys[GLFW_KEY_E] && player.cooldownE <= 0) {
            // player.isAccelerating = true; // 这个标志现在由 Update 内部管理
            player.accelTimer = 3.0f; // 启动加速计时器
            player.cooldownE = 15.0f;
            audioSystem.PlaySound("skill_e");
        }
        if (keys[GLFW_KEY_Q] && player.cooldownQ <= 0) {
            for (auto& m : monsters) m.frozen = true;
            player.cooldownQ = 20.0f;
            audioSystem.PlaySound("skill_q");
        }

        // --- Update Logic ---
        // player.Update(deltaTime); // 可以简化，因为大部分逻辑已移出或重构
        // 保留 Update 以处理技能计时器等
        if (player.accelTimer > 0) {
            player.accelTimer -= deltaTime;
            player.isAccelerating = true;
        }
        else {
            player.isAccelerating = false;
        }
        if (player.cooldownE > 0) player.cooldownE -= deltaTime;
        if (player.cooldownQ > 0) player.cooldownQ -= deltaTime;


        // --- Monster 和 Collectible 逻辑 (基本保持不变) ---
        alertTriggered = false;
        for (auto& monster : monsters) {
            monster.Update(deltaTime, player, mazeGen, CELL_SIZE);
            if (!alertTriggered && monster.visible && glm::distance(monster.position, player.position) < monster.detectionRange) {
                alertTriggered = true;
                audioSystem.PlaySound("alert");
            }
            if (monster.frozen && player.cooldownQ <= (20.0f - 2.0f + 0.1f)) {
                monster.frozen = false;
            }
        }

        for (auto& item : collectibles) {
            if (!item.collected) {
                float dx = player.position.x - item.position.x;
                float dy = player.position.y - item.position.y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance < (player.radius + item.size / 2)) {
                    item.collected = true;
                    score--;
                    audioSystem.PlaySound("collect");
                }
            }
        }

        // --- 胜利条件检查 ---
        if (score <= 0 && !gameWon) {
            gameWon = true;
            victoryTimer = 0.0f;
            std::cout << "Victory! Game will restart shortly...\n";
        }

        // --- 胜利状态处理 ---
        if (gameWon) {
            victoryTimer += deltaTime;
            if (victoryTimer >= victoryDisplayTime) {
                ResetGame(mazeGen, player, monsters, collectibles, score, MAZE_WIDTH, MAZE_HEIGHT, CELL_SIZE);
                gameWon = false;
                victoryTimer = 0.0f;
            }
        }

        // --- 渲染 ---
        renderer.BeginFrame();

        if (alertTriggered) {
            renderer.DrawAlertFlash();
            renderer.BeginFrame();
        }

        renderer.DrawMaze(mazeGen, CELL_SIZE);
        renderer.DrawCollectibles(collectibles);
        renderer.DrawMonsters(monsters);
        renderer.DrawPlayer(player); // Player 的 position 会被正确更新用于渲染

        renderer.EndFrame(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// 回调函数实现
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 键盘回调函数
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

// 鼠标按钮回调函数
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        // std::cout << "Mouse clicked at: " << xpos << ", " << ypos << std::endl;
        // 在这里处理 UI 点击，例如重启按钮
    }
}