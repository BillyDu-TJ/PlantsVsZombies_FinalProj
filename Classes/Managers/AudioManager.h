// ��Ƶ������ - ͳһ����������ֺ���Ч
// 2025.12.15 by BillyDu
#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "cocos2d.h"
#include "editor-support/cocostudio/SimpleAudioEngine.h"

class AudioManager {
public:
    static AudioManager& getInstance();
    
    // �������ֿ���
    void playBackgroundMusic(const std::string& filename, bool loop = true);
    void stopBackgroundMusic();
    void pauseBackgroundMusic();
    void resumeBackgroundMusic();
    
    // ��Ч����
    void playEffect(const std::string& filename);
    
    // ��������
    void setMusicVolume(float volume);      // 0.0f - 1.0f
    void setEffectVolume(float volume);     // 0.0f - 1.0f
    float getMusicVolume() const;
    float getEffectVolume() const;
    
    // ���ؿ���
    void setMusicEnabled(bool enabled);
    void setEffectEnabled(bool enabled);
    bool isMusicEnabled() const { return _musicEnabled; }
    bool isEffectEnabled() const { return _effectEnabled; }
    
    // Ԥ������Ƶ�ļ�
    void preloadAudio();
    
private:
    AudioManager();
    ~AudioManager();
    
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    
    CocosDenshion::SimpleAudioEngine* _audioEngine;
    bool _musicEnabled = true;
    bool _effectEnabled = true;
    float _musicVolume = 0.7f;
    float _effectVolume = 1.0f;
};

#endif // __AUDIO_MANAGER_H__