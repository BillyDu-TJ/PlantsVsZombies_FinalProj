// ÒôÆµ¹ÜÀíÆ÷ÊµÏÖ
// 2025.12.15 by BillyDu
#include "AudioManager.h"
#include "../Consts.h"

USING_NS_CC;

AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager() {
    _audioEngine = CocosDenshion::SimpleAudioEngine::getInstance();
}

AudioManager::~AudioManager() {
    _audioEngine->end();
}

void AudioManager::playBackgroundMusic(const std::string& filename, bool loop) {
    if (!_musicEnabled) return;
    
    _audioEngine->playBackgroundMusic(filename.c_str(), loop);
    _audioEngine->setBackgroundMusicVolume(_musicVolume);
    
    CCLOG("[Info] Playing BGM: %s", filename.c_str());
}

void AudioManager::stopBackgroundMusic() {
    _audioEngine->stopBackgroundMusic();
}

void AudioManager::pauseBackgroundMusic() {
    _audioEngine->pauseBackgroundMusic();
}

void AudioManager::resumeBackgroundMusic() {
    _audioEngine->resumeBackgroundMusic();
}

void AudioManager::playEffect(const std::string& filename) {
    if (!_effectEnabled) return;
    
    _audioEngine->playEffect(filename.c_str());
    CCLOG("[Info] Playing Effect: %s", filename.c_str());
}

void AudioManager::setMusicVolume(float volume) {
    _musicVolume = cocos2d::clampf(volume, 0.0f, 1.0f);
    _audioEngine->setBackgroundMusicVolume(_musicVolume);
}

void AudioManager::setEffectVolume(float volume) {
    _effectVolume = cocos2d::clampf(volume, 0.0f, 1.0f);
    _audioEngine->setEffectsVolume(_effectVolume);
}

float AudioManager::getMusicVolume() const {
    return _musicVolume;
}

float AudioManager::getEffectVolume() const {
    return _effectVolume;
}

void AudioManager::setMusicEnabled(bool enabled) {
    _musicEnabled = enabled;
    if (!enabled) {
        pauseBackgroundMusic();
    }
}

void AudioManager::setEffectEnabled(bool enabled) {
    _effectEnabled = enabled;
}

void AudioManager::preloadAudio() {
    // Ô¤¼ÓÔØ±³¾°ÒôÀÖ
    _audioEngine->preloadBackgroundMusic(AudioPath::MAIN_MENU_BGM);
    _audioEngine->preloadBackgroundMusic(AudioPath::GAME_BGM);
    
    // Ô¤¼ÓÔØÒôÐ§
    _audioEngine->preloadEffect(AudioPath::PLANT_SOUND);
    _audioEngine->preloadEffect(AudioPath::SHOOT_SOUND);
    _audioEngine->preloadEffect(AudioPath::ZOMBIE_DIE_SOUND);
    _audioEngine->preloadEffect(AudioPath::VICTORY_SOUND);
    _audioEngine->preloadEffect(AudioPath::LOSE_SOUND);
    
    CCLOG("[Info] Audio files preloaded successfully.");
}