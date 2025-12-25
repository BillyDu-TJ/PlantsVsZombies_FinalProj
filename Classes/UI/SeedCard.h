// 种子卡片类头文件
// 2025.12.15 by BillyDu
// ���£����ֲ����ֲ��ȴ���ܣ�������ȴ����ʱ��ʾ��״̬����
// by Zhao.12.23
#ifndef __SEED_CARD_H__
#define __SEED_CARD_H__

#include "cocos2d.h"
#include <functional>
#include <string>

class SeedCard : public cocos2d::Node {
public:
    static SeedCard* create(int plantId);
    
    // ����ѡ��ص�
    void setOnSelectCallback(const std::function<void(int)>& callback);
    
    // 更新阳光是否足够 (用于改变卡片颜色/禁用状态)
    void updateSunCheck(int currentSun);
    
    // ������ȴ״̬
    void updateCooldown(float dt);
    
    // �����ȴ
    void startCooldown(float cooldownTime);
    
    // ����Ƿ�����ȴ��
    bool isInCooldown() const { return _cooldownRemaining > 0.0f; }
    
    int getPlantId() const { return _plantId; }

private:
    bool init(int plantId);
    
    int _plantId;
    int _cost;
    
    // ��ȴ���
    float _cooldownRemaining = 0.0f;  // ʣ����ȴʱ��
    float _cooldownTotal = 0.0f;      // ����ȴʱ��
    
    // UI ���
    cocos2d::Sprite* _bg = nullptr;        // 卡片背景PNG素材，
    cocos2d::Sprite* _icon = nullptr;      // ֲ��ͼ�꣨�����Ҫ��
    cocos2d::Label* _costLabel = nullptr;  // 价格标签（备用）
    cocos2d::Label* _cooldownLabel = nullptr;  // 冷却倒计时标签
    
    std::function<void(int)> _onSelectCallback;
};

#endif // __SEED_CARD_H__