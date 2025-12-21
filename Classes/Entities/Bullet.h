// 子弹类定义文件
// 2025.12.12 by BillyDu 12.21 by Zhao
#ifndef __BULLET_H__
#define __BULLET_H__

#include "Unit.h"
#include "GameDataStructures.h"

class Bullet : public Unit {
public:
    static Bullet* create(const BulletData& data);
    virtual bool init() override;
    virtual void updateLogic(float dt) override;

    void setBulletData(const BulletData& data);

    // 获取伤害值
    int getDamage() const { return _data.damage; }
    
    // 获取子弹类型
    BulletType getType() const { return _data.type; }
    
    // 获取减速效果倍数
    float getSlowEffect() const { return _data.slowEffect; }

    // 标记子弹是否已经击中（防止一次穿透多个敌人）
    bool isActive() const { return _active; }
    void deactivate() { _active = false; }

private:
    BulletData _data;
    bool _active = true;
};

#endif // __BULLET_H__