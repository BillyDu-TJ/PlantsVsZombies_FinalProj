// 植物类定义，继承自Unit类，包含植物特有的属性和方法
// 2025.12.2 by BillyDu
//12.22 by Zhao ����ֲ�￪���������������ǵ��޸Ļ�ȥ��
#ifndef __PLANT_H__
#define __PLANT_H__

#include <functional>
#include <string>

#include "Unit.h"
#include "GameDataStructures.h" // 包含之前定义的数据结构

class Plant : public Unit {
public:
    static Plant* createWithData(const PlantData& data);
    virtual bool init() override;

    // 重写父类函数
    virtual void updateLogic(float dt) override;
    virtual void die() override;

    // ֲ�������߼�
    void setPlantData(const PlantData& data);

    // 触发技能，如发射豌豆、生产阳光等
    virtual void triggerSkill();

    // 设置发射子弹的回调：位置 (Vec2), 伤害 (int) - 也可以传 BulletData
    void setOnShootCallback(const std::function<void(cocos2d::Vec2, int)>& callback);

    // ���ü���Ƿ��й���Ŀ��Ļص�
    void setHasTargetCallback(const std::function<bool()>& callback);
    
    // ������ط���
    void playAnimation(const std::string& animName);  // ����ָ������
    void playDefaultAnimation();  // ����Ĭ�϶���

    // --- ֻ�������� ---
    const std::string& getName() const { return _data.name; }
    const std::string& getType() const { return _data.type; }
    const PlantData& getData() const { return _data; }

protected:
    PlantData _data;
    float _timer; // 用于攻击/生产间隔的计时器
    std::function<void(cocos2d::Vec2, int)> _onShootCallback;
    std::function<bool()> _hasTargetCallback;  // ����Ƿ��й���Ŀ��Ļص�
    std::string _currentAnimation;  // ��ǰ���ŵĶ�������
};

#endif // __PLANT_H__