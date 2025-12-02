// 实现僵尸类
// 2025.12.2 by BillyDu
#include "Zombie.h"
#include "../Consts.h" // 需要知道边界信息

USING_NS_CC;

bool Zombie::init() {
    if (!Unit::init()) return false;

    _type = UnitType::ZOMBIE;
    _state = UnitState::WALK;
    _speed = 10.0f; // 默认速度，会被 DataManager 覆盖
    _damage = 10;

    // 暂时给个临时贴图或颜色块，防止看不见
    this->setTexture("zombies/normalzombie.png"); // 确保你有这个图，或者用 SetColor
    if (this->getContentSize().width == 0) {
        // 如果图没加载到，画个红块代替
        this->setTextureRect(Rect(0, 0, 60, 90));
        this->setColor(Color3B::RED);
    }

    return true;
}

void Zombie::updateLogic(float dt) {
    Unit::updateLogic(dt);

    if (_state == UnitState::WALK) {
        // 向左移动
        float moveDist = _speed * dt;
        this->setPositionX(this->getPositionX() - moveDist);

        // 简单的越界检查 (如果走出屏幕左边，游戏结束或销毁)
        if (this->getPositionX() < -50) {
            this->removeFromParent();
            CCLOG("[Info] Zombie reached the house! Game Over?");
        }
    }
    else if (_state == UnitState::ATTACK) {
        // 攻击逻辑 (待实现)
    }
}