#pragma once
// 注意：你需要将 miniaudio.c 添加到你的项目源文件中进行编译
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <map>
#include <string>
#include <iostream>

// 音频系统类
class AudioSystem {
private:
	std::map<std::string, ma_engine*> engines;// 支持多个音频引擎
	std::map<std::string, ma_sound> sounds; // 存储加载的音效

public:
	// 构造函数，初始化音频引擎
    AudioSystem() {
        ma_result result;
        ma_engine* pEngine = new ma_engine;
        result = ma_engine_init(NULL, pEngine);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to initialize audio engine." << std::endl;
            delete pEngine;
            return;
        }
        engines["default"] = pEngine;
    }

    ~AudioSystem() {
        for (auto& pair : sounds) {
            ma_sound_uninit(&pair.second);
        }
        for (auto& pair : engines) {
            ma_engine_uninit(pair.second);
            delete pair.second;
        }
    }

	// 加载音效文件
    void LoadSound(const std::string& name, const std::string& filepath) {
        if (engines.find("default") == engines.end()) return;
        ma_result result = ma_sound_init_from_file(engines["default"], filepath.c_str(), 0, NULL, NULL, &sounds[name]);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to load sound: " << filepath << std::endl;
        }
    }

	// 播放音效
    void PlaySound(const std::string& name) {
        auto it = sounds.find(name);
        if (it != sounds.end()) {
            ma_sound_start(&it->second);
        }
    }
};