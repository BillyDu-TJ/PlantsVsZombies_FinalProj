// 僵尸的类定义，继承自Unit类，包含僵尸特有的属性和方法
// 2025.12.2 by BillyDu
#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__

#include "Unit.h"

class Zombie : public Unit {
public:
    CREATE_FUNC(Zombie);
    virtual bool init() override;

    virtual void updateLogic(float dt) override;

    // 僵尸特有
    void setSpeed(float speed) { _speed = speed; }
    void setAttackDamage(int dmg) { _damage = dmg; }

private:
    float _speed;
    int _damage;
    // float _attackInterval; // 攻击间隔
};

#endif // __ZOMBIE_H__