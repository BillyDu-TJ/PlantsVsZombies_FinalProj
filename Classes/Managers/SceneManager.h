// 场景管理器 - 统一管理场景切换
// 2025.12.15 by BillyDu
#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "cocos2d.h"
#include "../Consts.h"

class SceneManager {
public:
    static SceneManager& getInstance();
    
    // 场景切换方法
    void gotoStartScene();
    void gotoGameScene();
    void gotoVictoryScene();
    void gotoGameOverScene();
    
    // 游戏状态管理
    void setGameState(GameState state) { _currentState = state; }
    GameState getGameState() const { return _currentState; }
    
    // 重新开始游戏
    void restartGame();
    
private:
    SceneManager() = default;
    ~SceneManager() = default;
    
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    
    GameState _currentState = GameState::MENU;
    
    // 辅助方法：安全的场景切换
    void replaceSceneWithTransition(cocos2d::Scene* scene);
};

#endif // __SCENE_MANAGER_H__