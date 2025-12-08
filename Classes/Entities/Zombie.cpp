// 实现僵尸类
// 2025.12.2 by BillyDu
#include "Zombie.h"
#include "../Consts.h" // 需要知道边界信息

USING_NS_CC;

Zombie* Zombie::createWithData(const ZombieData& data) {
    Zombie* pRet = new(std::nothrow) Zombie();
    if (pRet && pRet->init()) {
        pRet->setZombieData(data); // 关键：在这里注入数据
        pRet->autorelease();
        return pRet;
    }
    else {
        delete pRet;
        return nullptr;
    }
}

bool Zombie::init() {
    if (!Unit::init()) return false;

    _type = UnitType::ZOMBIE;
    _state = UnitState::WALK;
    _attackTimer = 0.0f;

    return true;
}

// 数据注入逻辑
void Zombie::setZombieData(const ZombieData& data) {
    _data = data;

    // 1. 设置基类属性
    this->setHp(data.hp);

    // 2. 设置贴图 (带兜底逻辑)
    bool textureLoaded = false;
    if (!_data.texturePath.empty()) {
        if (FileUtils::getInstance()->isFileExist(_data.texturePath)) {
            this->setTexture(_data.texturePath);
            textureLoaded = true;
        }
        else {
            CCLOG("[Err] Zombie texture missing: %s", _data.texturePath.c_str());
        }
    }

    // 兜底红块
    if (!textureLoaded) {
        this->setTextureRect(Rect(0, 0, 60, 90));
        this->setColor(Color3B::RED);
        CCLOG("[Warn] Zombie %s used fallback color block.", _data.name.c_str());
    }
}

void Zombie::updateLogic(float dt) {
    Unit::updateLogic(dt);

    if (_state == UnitState::WALK) {
        // 向左移动
        float moveDist = _data.speed * dt;
        this->setPositionX(this->getPositionX() - moveDist);

        // 简单的越界检查 (如果走出屏幕左边，游戏结束或销毁)
        if (this->getPositionX() < -50) {
            this->removeFromParent();
            CCLOG("[Info] Zombie reached the house! Game Over?");
			// TODO: 触发游戏结束逻辑
        }
    }
    else if (_state == UnitState::ATTACK) {
        _attackTimer += dt;
        if (_attackTimer >= _data.attackInterval) {
            _attackTimer = 0;
            // TODO: execute attack
            CCLOG("[Info] Zombie %s attacks! Damage: %d", _data.name.c_str(), _data.damage);
        }
    }
}