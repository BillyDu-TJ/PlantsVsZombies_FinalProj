// 僵尸的类定义，继承自Unit类，包含僵尸特有的属性和方法
// 2025.12.2 by BillyDu
#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__

#include "Unit.h"
#include "GameDataStructures.h"

class Zombie : public Unit {
public:
    static Zombie* createWithData(const ZombieData& data);
    virtual bool init() override;

    virtual void updateLogic(float dt) override;

    void setZombieData(const ZombieData& data);

    // 检查是否攻击冷却完毕
    bool canAttack() const;

    // 重置攻击计时器
    void resetAttackTimer();

    // 获取攻击力
    int getDamage() const { return _data.damage; }

private:
    ZombieData _data;
	float _attackTimer = 0.0f;
};

#endif // __ZOMBIE_H__