// 植物的类定义，继承自Unit类，包含植物特有的属性和方法
// 2025.12.2 by BillyDu
#ifndef __PLANT_H__
#define __PLANT_H__

#include <functional>

#include "Unit.h"
#include "GameDataStructures.h" // 引用之前定义的数据结构

class Plant : public Unit {
public:
    static Plant* createWithData(const PlantData& data);
    virtual bool init() override;

    // 覆写基类方法
    virtual void updateLogic(float dt) override;
    virtual void die() override;

    // 植物特有逻辑
    void setPlantData(const PlantData& data);

    // 触发技能（如发射豌豆、生产阳光）
    virtual void triggerSkill();

    // 参数：子弹产生的位置 (Vec2), 伤害 (int) - 也可以传 BulletData
    void setOnShootCallback(const std::function<void(cocos2d::Vec2, int)>& callback);

protected:
    PlantData _data;
    float _timer; // 用于攻击或生产的计时器
    std::function<void(cocos2d::Vec2, int)> _onShootCallback;
};

#endif // __PLANT_H__