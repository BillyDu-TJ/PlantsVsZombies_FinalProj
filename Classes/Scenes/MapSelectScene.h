// 地图选择场景
// 2025.12.21 Zhao
#ifndef __MAP_SELECT_SCENE_H__
#define __MAP_SELECT_SCENE_H__

#include "cocos2d.h"

class MapSelectScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    
    CREATE_FUNC(MapSelectScene);
    
private:
    // 按钮回调函数
    void onMapButtonClicked(cocos2d::Ref* sender, int mapId);
    void onBackButtonClicked(cocos2d::Ref* sender);
    
    // UI 创建
    void createBackground();
    void createTitle();
    void createMapButtons();
    void createBackButton();
    
    // 成员变量
    cocos2d::Size _visibleSize;
    cocos2d::Vec2 _origin;
    
    // 地图配置：地图ID -> 背景路径
    std::string getMapBackgroundPath(int mapId) const;
    bool isMapUnlocked(int mapId) const;
};

#endif // __MAP_SELECT_SCENE_H__

