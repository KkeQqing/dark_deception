// src/AudioSystem.cpp
#include "AudioSystem.h"

AudioSystem::AudioSystem() {
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine." << std::endl;
    }
}

AudioSystem::~AudioSystem() {
    for (auto& pair : sounds) {
        ma_sound_uninit(&pair.second);
    }
    ma_sound_uninit(&music);
    ma_engine_uninit(&engine);
}

bool AudioSystem::loadSound(const std::string& name, const std::string& filepath) {
    ma_sound sound;
    ma_result result = ma_sound_init_from_file(&engine, filepath.c_str(), 0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load sound: " << filepath << std::endl;
        return false;
    }
    sounds[name] = sound;
    return true;
}

void AudioSystem::playSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        ma_sound_start(&it->second);
    }
}

bool AudioSystem::loadMusic(const std::string& filepath) {
    ma_result result = ma_sound_init_from_file(&engine, filepath.c_str(), 0, NULL, NULL, &music);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load music: " << filepath << std::endl;
        return false;
    }
    ma_sound_set_looping(&music, MA_TRUE);
    return true;
}

void AudioSystem::playMusic() {
    ma_sound_start(&music);
}

void AudioSystem::stopMusic() {
    ma_sound_stop(&music);
}