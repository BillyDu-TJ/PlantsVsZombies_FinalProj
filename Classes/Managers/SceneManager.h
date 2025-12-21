// 场景管理器 - 统一管理场景的切换
// 2025.12.15 by BillyDu
#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "cocos2d.h"
#include "../Consts.h"

class SceneManager {
public:
    static SceneManager& getInstance();
    
    // 场景切换函数
    void gotoStartScene();
    void gotoMapSelectScene();
    void gotoPlantSelectScene();
    void gotoGameScene();
    void gotoVictoryScene();
    void gotoGameOverScene();
    
    // 植物选择数据传递
    void setSelectedPlants(const std::vector<int>& plantIds) { _selectedPlantIds = plantIds; }
    const std::vector<int>& getSelectedPlants() const { return _selectedPlantIds; }
    
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
    
    // 植物选择数据
    std::vector<int> _selectedPlantIds;
    
    // 统一的场景切换，带有过渡效果
    void replaceSceneWithTransition(cocos2d::Scene* scene);
};

#endif // __SCENE_MANAGER_H__
