// ����ඨ��
//  2025.12.12 by BillyDu
#ifndef __SUN_H__
#define __SUN_H__

#include "cocos2d.h"
#include <functional>

class Sun : public cocos2d::Sprite {
public:
    static Sun* create();
    virtual bool init() override;

    // �����ռ��ص��������ɵ����ϽǺ���ã�
    void setOnCollectedCallback(const std::function<void(int)>& callback);

    // ����ģʽ 1: �������
    // targetY: ���䵽�ĵ���߶�
    void fallFromSky(float startX, float targetY);

    // ����ģʽ 2: ��ֲ����� (��������Ծ)
    void jumpFromPlant(cocos2d::Vec2 startPos, cocos2d::Vec2 targetPos);

private:
    // �������ռ��߼�
    void collect();

    bool _isCollected = false; // ��ֹ�ظ����
    std::function<void(int)> _onCollectedCallback;
    int _value = 25; // һ����� 25 ��
};

#endif // __SUN_H__