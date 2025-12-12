// 实现植物类
// 2025.12.2 by BillyDu
#include "Plant.h"

USING_NS_CC;

Plant* Plant::createWithData(const PlantData& data) {
    Plant* pRet = new(std::nothrow) Plant();
    if (pRet && pRet->init()) {
        pRet->setPlantData(data); // 初始化数据
        pRet->autorelease();
        return pRet;
    }
    else {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool Plant::init() {
    if (!Unit::init()) return false;

    _type = UnitType::PLANT;
    _timer = 0.0f;
    return true;
}

void Plant::setPlantData(const PlantData& data) {
    _data = data;
    this->setHp(data.hp);

    // 加载纹理 (如果文件存在)
    if (!_data.texturePath.empty()) {
        this->setTexture(_data.texturePath);
    }
}

void Plant::updateLogic(float dt) {
    Unit::updateLogic(dt);

    // 简单的冷却逻辑
    if (_data.attackSpeed > 0) {
        _timer += dt;

        if (_timer >= _data.attackSpeed) {
            // 注意：这里用 attackSpeed 暂代 攻击/生产 间隔
            CCLOG("[Debug] Plant Timer Reached! (Timer: %.2f, Speed: %.2f)", _timer, _data.attackSpeed);
            _timer = 0;
            triggerSkill();
        } else {
            // [调试检查 3] 如果 attackSpeed 是 0，报个错
            // 限制打印频率，防止刷屏
            static bool hasWarned = false;
            if (!hasWarned) {
                CCLOG("[Err] Plant %s has 0 or negative attackSpeed: %f", _data.name.c_str(), _data.attackSpeed);
                hasWarned = true;
            }
        }
    }
}

void Plant::triggerSkill() {
    CCLOG("[Debug] triggerSkill called for: %s (Type: %s)", _data.name.c_str(), _data.type.c_str());

    // 可以在这里根据 _data.type 判断行为
    if (_data.type == "shooter") {
        CCLOG("[Info] Plant %s shoots!", _data.name.c_str());

        if (_onShootCallback) {
            // 子弹发射位置：植物中心向右一点，向上一点(嘴巴位置)
            Vec2 spawnPos = this->getPosition() + Vec2(20, 10);
            _onShootCallback(spawnPos, _data.attack);
        } else {
            CCLOG("[Err] _onShootCallback is NULL! Did you set it in GameScene?");
        }
    }
    else if (_data.type == "producer") {
        // ...
    } 
    else {
        CCLOG("[Err] Plant type mismatch! Expected 'shooter', got '%s'", _data.type.c_str());
    }
}

void Plant::die() {
    CCLOG("Plant %s died!", _data.name.c_str());
    Unit::die();
}

void Plant::setOnShootCallback(const std::function<void(cocos2d::Vec2, int)>& callback) {
    _onShootCallback = callback;
}
