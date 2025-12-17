// 场景管理器实现
// 2025.12.15 by BillyDu
#include "SceneManager.h"
#include "../Scenes/StartScene.h"
#include "../Scenes/GameScene.h"
#include "../Scenes/VictoryScene.h"
#include "../Scenes/GameOverScene.h"
#include "AudioManager.h"

USING_NS_CC;

SceneManager& SceneManager::getInstance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::gotoStartScene() {
    _currentState = GameState::MENU;
    auto scene = StartScene::createScene();
    replaceSceneWithTransition(scene);
    
    // 播放主菜单音乐
    AudioManager::getInstance().playBackgroundMusic(AudioPath::MAIN_MENU_BGM);
}

void SceneManager::gotoGameScene() {
    _currentState = GameState::PLAYING;
    auto scene = GameScene::createScene();
    replaceSceneWithTransition(scene);
    
    // 播放游戏音乐
    AudioManager::getInstance().playBackgroundMusic(AudioPath::GAME_BGM);
}

void SceneManager::gotoVictoryScene() {
    _currentState = GameState::VICTORY;
    auto scene = VictoryScene::createScene();
    replaceSceneWithTransition(scene);
    
    // 播放胜利音效
    AudioManager::getInstance().playEffect(AudioPath::VICTORY_SOUND);
}

void SceneManager::gotoGameOverScene() {
    _currentState = GameState::GAME_OVER;
    auto scene = GameOverScene::createScene();
    replaceSceneWithTransition(scene);
    
    // 播放失败音效
    AudioManager::getInstance().playEffect(AudioPath::LOSE_SOUND);
}

void SceneManager::restartGame() {
    CCLOG("[Info] Restarting game...");
    gotoGameScene();
}

void SceneManager::replaceSceneWithTransition(cocos2d::Scene* scene) {
    auto director = Director::getInstance();
    
    // 使用淡入淡出过渡效果
    auto transition = TransitionFade::create(0.5f, scene);
    director->replaceScene(transition);
    
    CCLOG("[Info] Scene transition completed.");
}