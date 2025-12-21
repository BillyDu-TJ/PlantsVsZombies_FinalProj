// 植物选择场景
//12.21 Zhao
//这里有点问题就是对于鼠标事件的监听不是很灵敏，同时可能需要美化一下select的界面
#ifndef __PLANT_SELECT_SCENE_H__
#define __PLANT_SELECT_SCENE_H__

#include "cocos2d.h"
#include <vector>
#include <string>
#include <map>
#include <algorithm>

class PlantSelectScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    
    CREATE_FUNC(PlantSelectScene);
    
private:
    // UI 创建
    void createBackground();
    void createTitle();
    void createCardArea();
    void createConfirmButton();
    void createGlobalMouseListener();
    
    // 鼠标点击选择卡片
    void onCardLeftClick(cocos2d::EventMouse* event, cocos2d::Node* card); // 左键：选择卡片
    void onCardRightClick(cocos2d::EventMouse* event, cocos2d::Node* card); // 右键：取消选择
    
    
    // 按钮回调
    void onConfirmButtonClicked(cocos2d::Ref* sender);
    void onBackButtonClicked(cocos2d::Ref* sender);
    
    // 成员变量
    cocos2d::Size _visibleSize;
    cocos2d::Vec2 _origin;
    
    // 选中的植物
    std::vector<int> _selectedPlantIds; // 选中的植物ID列表（最多8个）
    cocos2d::Label* _selectedCountLabel = nullptr; // 显示已选择数量的标签
    
    // 卡片区域
    cocos2d::Node* _cardArea = nullptr;
    std::vector<cocos2d::Node*> _availableCards; // 所有可用卡片
    
    // 卡片选择状态映射：卡片节点 -> 是否已选中
    std::map<cocos2d::Node*, bool> _cardSelectedMap;
    cocos2d::EventListenerMouse* _globalMouseListener = nullptr;
    
    // 最大卡片数量
    static const int MAX_SELECTED_CARDS = 8;
};

#endif // __PLANT_SELECT_SCENE_H__

