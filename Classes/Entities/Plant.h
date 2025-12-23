// 植物类定义，继承自Unit类，包含植物特有的属性和方法
// 2025.12.2 by BillyDu
//12.22 by Zhao 神人植物开挂提升攻击力（记得修改回去）
#ifndef __PLANT_H__
#define __PLANT_H__

#include <functional>
#include <string>

#include "Unit.h"
#include "GameDataStructures.h" // 包含之前定义的数据结构

class Plant : public Unit {
public:
    static Plant* createWithData(const PlantData& data);
    virtual bool init() override;

    // 重写父类函数
    virtual void updateLogic(float dt) override;
    virtual void die() override;

    // 植物特有逻辑
    void setPlantData(const PlantData& data);

    // 触发技能，如发射豌豆、生产阳光等
    virtual void triggerSkill();

    // 设置发射子弹的回调：位置 (Vec2), 伤害 (int) - 也可以传 BulletData
    void setOnShootCallback(const std::function<void(cocos2d::Vec2, int)>& callback);
    
    // 设置检查是否有攻击目标的回调
    void setHasTargetCallback(const std::function<bool()>& callback);
    
    // 动画相关方法
    void playAnimation(const std::string& animName);  // 播放指定动画
    void playDefaultAnimation();  // 播放默认动画

    // --- 只读访问器 ---
    const std::string& getName() const { return _data.name; }
    const std::string& getType() const { return _data.type; }
    const PlantData& getData() const { return _data; }

protected:
    PlantData _data;
    float _timer; // 用于攻击/生产间隔的计时器
    std::function<void(cocos2d::Vec2, int)> _onShootCallback;
    std::function<bool()> _hasTargetCallback;  // 检查是否有攻击目标的回调
    std::string _currentAnimation;  // 当前播放的动画名称
};

#endif // __PLANT_H__
