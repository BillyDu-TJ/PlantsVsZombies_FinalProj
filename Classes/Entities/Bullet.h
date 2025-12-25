// �ӵ��ඨ���ļ�
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

    // ��ȡ�˺�ֵ
    int getDamage() const { return _data.damage; }

    // ��ȡ�ӵ�����
    BulletType getType() const { return _data.type; }
    
    // ��ȡ����Ч������
    float getSlowEffect() const { return _data.slowEffect; }

    // ����ӵ��Ƿ��Ѿ����У���ֹһ�δ�͸������ˣ�
    bool isActive() const { return _active; }
    void deactivate() { _active = false; }

private:
    BulletData _data;
    bool _active = true;
};

#endif // __BULLET_H__