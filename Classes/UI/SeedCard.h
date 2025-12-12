// 种植卡片类定义
// 2025.12.12 by BillyDu
#ifndef __SEED_CARD_H__
#define __SEED_CARD_H__

#include "cocos2d.h"
#include <functional>
#include <string>

class SeedCard : public cocos2d::Node {
public:
    static SeedCard* create(int plantId);
    bool init(int plantId);

    // 设置点击回调
    void setOnSelectCallback(const std::function<void(int)>& callback);

    // 检查阳光是否足够 (用于改变卡片置灰/变亮状态)
    void updateSunCheck(int currentSun);

    int getPlantId() const { return _plantId; }

private:
    int _plantId;
    int _cost;
    cocos2d::Sprite* _bg;      // 卡片背景
    cocos2d::Sprite* _icon;    // 植物图标
    cocos2d::Label* _costLabel; // 阳光消耗文字

    std::function<void(int)> _onSelectCallback;
};

#endif // __SEED_CARD_H__