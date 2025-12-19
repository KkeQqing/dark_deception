// src/AudioSystem.h
#ifndef AUDIOSYSTEM_H
#define AUDIOSYSTEM_H

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <unordered_map>
#include <string>
#include <iostream>

class AudioSystem {
private:
    ma_engine engine;
    std::unordered_map<std::string, ma_sound> sounds;
    ma_sound music;

public:
    AudioSystem();
    ~AudioSystem();

    bool loadSound(const std::string& name, const std::string& filepath);
    void playSound(const std::string& name);
    bool loadMusic(const std::string& filepath);
    void playMusic();
    void stopMusic();
};

#endif