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

private:
    ZombieData _data;
	float _attackTimer = 0.0f;
};

#endif // __ZOMBIE_H__