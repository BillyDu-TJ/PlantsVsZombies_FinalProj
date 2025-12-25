// 阳光类定义
//  2025.12.12 by BillyDu
#ifndef __SUN_H__
#define __SUN_H__

#include "cocos2d.h"
#include <functional>

class Sun : public cocos2d::Sprite {
public:
    static Sun* create();
    virtual bool init() override;

    // 设置收集回调（当阳光飞到左上角后调用）
    void setOnCollectedCallback(const std::function<void(int)>& callback);

    // 动作模式 1: 从天而降
    // targetY: 掉落到的地面高度
    void fallFromSky(float startX, float targetY);

    // 动作模式 2: 从植物产出 (抛物线跳跃)
    void jumpFromPlant(cocos2d::Vec2 startPos, cocos2d::Vec2 targetPos);

private:
    // 处理点击收集逻辑
    void collect();

    bool _isCollected = false; // 防止重复点击
    std::function<void(int)> _onCollectedCallback;
    int _value = 25; // 一个阳光 25 点
};

#endif // __SUN_H__