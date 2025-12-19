#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>

// ===== stb_image: 图像加载 =====
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ===== miniaudio: 音频播放 =====
#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_MP3  
#include "miniaudio.h"

// ===== 着色器源码 =====
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}
)";

// ===== 工具函数：编译着色器 =====
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return id;
}

// ===== 主函数 =====
int main() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "Image + Audio Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // ========================
    // 图像加载与纹理创建
    // ========================
    const char* imagePath = "D:/C++/dark_deception/asset/img/test.png";
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char* data = stbi_load(imagePath, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load image: " << imagePath << std::endl;
        glfwTerminate();
        return -1;
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // ========================
    // 着色器程序
    // ========================
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ========================
    // 顶点数据（全屏四边形）
    // ========================
    float vertices[] = {
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // ========================
    // 音频初始化（仅一次！）
    // ========================
    ma_engine engine;
    ma_sound sound;
    bool audioLoaded = false;
    const char* audioPath = "D:\\C++\\dark_deception\\asset\\audio\\test.flac";

    std::ifstream testFile(audioPath, std::ios::binary);
    if (!testFile) {
        std::cerr << "ERROR: Audio file does not exist or cannot be opened!" << std::endl;
        return -1;
    }
    testFile.close();

    // 初始化并启动音频引擎
    if (ma_engine_init(NULL, &engine) == MA_SUCCESS) {
        ma_engine_start(&engine);

        // 加载并播放音频
        if (ma_sound_init_from_file(&engine, audioPath, 0, NULL, NULL, &sound) == MA_SUCCESS) {
            std::cout << "Audio loaded and playing..." << std::endl;
            ma_sound_start(&sound);
            audioLoaded = true;
        }
        else {
            std::cerr << "Failed to load audio file: " << audioPath << std::endl;
        }
    }
    else {
        std::cerr << "Failed to initialize audio engine!" << std::endl;
    }

    // ========================
    // 渲染主循环
    // ========================
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // 按空格键重播音频（防连发）
        static bool spacePressed = false;
        if (audioLoaded) {
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
                ma_sound_seek_to_pcm_frame(&sound, 0);
                ma_sound_start(&sound);
                spacePressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
                spacePressed = false;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ========================
    // 清理资源（不依赖 ma_engine_is_valid）
    // ========================
    if (audioLoaded) {
        ma_sound_uninit(&sound);
    }
    // 即使初始化失败，也可以安全调用 ma_engine_uninit（内部有保护）
    ma_engine_uninit(&engine);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);       
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &texture);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}