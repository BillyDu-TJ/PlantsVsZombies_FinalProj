// ������λ�࣬����ֲ���ʬ���ӵ��ȶ��̳��Դ���
// 2025.12.2 by BillyDu
#ifndef __UNIT_H__
#define __UNIT_H__

#include "cocos2d.h"

// ʹ��ǿ����ö�� (C++11 scoped enum)
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
    // ���캯���г�ʼ����Ա���� (C++11)
    Unit();
    virtual ~Unit() = default;

    // ������ʼ��
    virtual bool init() override;

    // --- ������Ϸ�߼��ӿ� (��̬) ---
    // �ܵ��˺�
    virtual void takeDamage(int damage);

    // �����߼� (���Ŷ������Ƴ��Լ���)
    virtual void die();

    // ÿ֡�߼����� (�ƶ�����������ʱ)
    virtual void updateLogic(float dt);

    // --- Getters & Setters ---
    // ʹ�� CC_SYNTHESIZE ��������� get/set��������д�Կ���Ȩ��
    // �к� (�����Ż���ײ���)
    CC_SYNTHESIZE(int, _row, Row);
    // ����
    CC_SYNTHESIZE(UnitType, _type, UnitType);
    // ״̬
    CC_SYNTHESIZE(UnitState, _state, State);

    // ����
    int getHp() const { return _hp; }
    void setHp(int hp) { _hp = hp; _maxHp = hp; } // ���ó�ʼѪ��ͬʱ�������Ѫ��

    bool isDead() const { return _hp <= 0; }

protected:
    int _hp;
    int _maxHp;
    // Ҳ�����Ҫ�ܻ�����Ч���ļ�ʱ����
};

#endif // __UNIT_H__