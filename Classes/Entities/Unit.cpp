// 实现 Unit 类
// 2025.12.2 by BillyDu
#include "Unit.h"

USING_NS_CC;

Unit::Unit()
    : _row(-1)
    , _type(UnitType::NONE)
    , _state(UnitState::IDLE)
    , _hp(0)
    , _maxHp(0)
{
}

bool Unit::init() {
    if (!Sprite::init()) {
        return false;
    }
    // 开启 update 调度，这样 update(dt) 会被引擎自动调用
    // 但我们可以定义自己的 updateLogic 供 GameManager 统一调用，或者直接用引擎的
    // 这里为了演示，我们先不 scheduleUpdate，由上层管理器统一控制逻辑可能更优
    return true;
}

void Unit::takeDamage(int damage) {
    if (isDead()) return;

    _hp -= damage;
    // 简单的受击反馈：变红一下
    this->setColor(Color3B::RED);
    // 使用 Lambda 延迟恢复颜色 (C++11)
    auto delay = DelayTime::create(0.1f);
    auto restore = CallFunc::create([this]() {
        this->setColor(Color3B::WHITE);
        });
    this->runAction(Sequence::create(delay, restore, nullptr));

    if (_hp <= 0) {
        die();
    }
}

void Unit::die() {
    _state = UnitState::DIE;
    // 暂时直接移除，以后可以播放死亡动画
    this->removeFromParent();
}

void Unit::updateLogic(float dt) {
    // 基类暂时没有通用逻辑
}