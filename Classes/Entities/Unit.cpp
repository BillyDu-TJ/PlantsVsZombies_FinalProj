// ʵ�� Unit ��
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
    // ���� update ���ȣ����� update(dt) �ᱻ�����Զ�����
    // �����ǿ��Զ����Լ��� updateLogic �� GameManager ͳһ���ã�����ֱ���������
    // ����Ϊ����ʾ�������Ȳ� scheduleUpdate�����ϲ������ͳһ�����߼����ܸ���
    return true;
}

void Unit::takeDamage(int damage) {
    if (isDead()) return;

    _hp -= damage;
    // �򵥵��ܻ����������һ��
    this->setColor(Color3B::RED);
    // ʹ�� Lambda �ӳٻָ���ɫ (C++11)
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
    if (_state == UnitState::DIE) return; // ��ֹ�ظ�����
    _state = UnitState::DIE;

    CCLOG("[Info] Unit died.");
    this->removeAllChildren(); // �Ƴ��ӽڵ㣨����صĺ�㣩
    this->removeFromParent();  // �ӳ����Ƴ�
}

void Unit::updateLogic(float dt) {
    // ������ʱû��ͨ���߼�
}