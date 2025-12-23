// 种子卡片类头文件
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
    
    // 更新阳光是否足够 (用于改变卡片颜色/禁用状态)
    void updateSunCheck(int currentSun);
    
    // 更新冷却状态
    void updateCooldown(float dt);
    
    // 启动冷却
    void startCooldown(float cooldownTime);
    
    // 检查是否在冷却中
    bool isInCooldown() const { return _cooldownRemaining > 0.0f; }
    
    int getPlantId() const { return _plantId; }

private:
    bool init(int plantId);
    
    int _plantId;
    int _cost;
    
    // 冷却相关
    float _cooldownRemaining = 0.0f;  // 剩余冷却时间
    float _cooldownTotal = 0.0f;      // 总冷却时间
    
    // UI 组件
    cocos2d::Sprite* _bg = nullptr;        // 卡片背景PNG素材，
    cocos2d::Sprite* _icon = nullptr;      // 植物图标（如果需要）
    cocos2d::Label* _costLabel = nullptr;  // 价格标签（备用）
    cocos2d::Label* _cooldownLabel = nullptr;  // 冷却倒计时标签
    
    std::function<void(int)> _onSelectCallback;
};

#endif // __SEED_CARD_H__
