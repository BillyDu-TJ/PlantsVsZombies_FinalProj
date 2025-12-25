// 基础单位类，所有植物、僵尸、子弹等都继承自此类
// 2025.12.2 by BillyDu
#ifndef __UNIT_H__
#define __UNIT_H__

#include "cocos2d.h"

// 使用强类型枚举 (C++11 scoped enum)
enum class UnitType {
    PLANT,
    ZOMBIE,
    BULLET,
    NONE
};

enum class UnitState {
    IDLE,
    WALK,
    ATTACK,
    DIE
};

class Unit : public cocos2d::Sprite {
public:
    // 构造函数中初始化成员变量 (C++11)
    Unit();
    virtual ~Unit() = default;

    // 基础初始化
    virtual bool init() override;

    // --- 核心游戏逻辑接口 (多态) ---
    // 受到伤害
    virtual void takeDamage(int damage);

    // 死亡逻辑 (播放动画、移除自己等)
    virtual void die();

    // 每帧逻辑更新 (移动、攻击倒计时)
    virtual void updateLogic(float dt);

    // --- Getters & Setters ---
    // 使用 CC_SYNTHESIZE 宏快速生成 get/set，或者手写以控制权限
    // 行号 (用于优化碰撞检测)
    CC_SYNTHESIZE(int, _row, Row);
    // 类型
    CC_SYNTHESIZE(UnitType, _type, UnitType);
    // 状态
    CC_SYNTHESIZE(UnitState, _state, State);

    // 属性
    int getHp() const { return _hp; }
    void setHp(int hp) { _hp = hp; _maxHp = hp; } // 设置初始血量同时设置最大血量

    bool isDead() const { return _hp <= 0; }

protected:
    int _hp;
    int _maxHp;
    // 也许还需要受击闪白效果的计时器等
};

#endif // __UNIT_H__