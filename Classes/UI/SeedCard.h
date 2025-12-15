// 种植卡片类头文件
// 2025.12.15 by BillyDu
#ifndef __SEED_CARD_H__
#define __SEED_CARD_H__

#include "cocos2d.h"
#include <functional>
#include <string>

class SeedCard : public cocos2d::Node {
public:
    static SeedCard* create(int plantId);
    
    // 设置选择回调
    void setOnSelectCallback(const std::function<void(int)>& callback);
    
    // 根据阳光是否足够 (用于改变卡片变灰/高亮状态)
    void updateSunCheck(int currentSun);
    
    int getPlantId() const { return _plantId; }

private:
    bool init(int plantId);
    
    int _plantId;
    int _cost;
    
    // UI 组件
    cocos2d::Sprite* _bg = nullptr;        // 卡片背景（PNG素材）
    cocos2d::Sprite* _icon = nullptr;      // 植物图标（如果需要）
    cocos2d::Label* _costLabel = nullptr;  // 消耗数字（备用）
    
    std::function<void(int)> _onSelectCallback;
};

#endif // __SEED_CARD_H__