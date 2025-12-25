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
    
    // ֲ��ѡ�����ݴ���
    void setSelectedPlants(const std::vector<int>& plantIds) { _selectedPlantIds = plantIds; }
    const std::vector<int>& getSelectedPlants() const { return _selectedPlantIds; }
    
    // ��Ϸ״̬����
    void setGameState(GameState state) { _currentState = state; }
    GameState getGameState() const { return _currentState; }
    
    // ���¿�ʼ��Ϸ
    void restartGame();
    
    // ��ǰ��ͼ���½ڣ�ID��1~4
    void setCurrentMapId(int mapId) { _currentMapId = mapId; }
    int getCurrentMapId() const { return _currentMapId; }
    
private:
    SceneManager() = default;
    ~SceneManager() = default;
    
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    
    GameState _currentState = GameState::MENU;
    
    // 当前地图（章节）ID：1=白天1，2=白天2，3=夜晚1，4=夜晚2
    int _currentMapId = 1;
    
    // 植物选择数据
    std::vector<int> _selectedPlantIds;
    
    // 统一的场景切换，带有过渡效果
    void replaceSceneWithTransition(cocos2d::Scene* scene);
};

#endif // __SCENE_MANAGER_H__