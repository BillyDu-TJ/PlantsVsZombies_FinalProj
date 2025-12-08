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
    if (_data.cooldown > 0) {
        _timer += dt;
        if (_timer >= _data.attackSpeed && _data.attackSpeed > 0) {
            // 注意：这里用 attackSpeed 暂代 攻击/生产 间隔
            _timer = 0;
            triggerSkill();
        }
    }
}

void Plant::triggerSkill() {
    // 可以在这里根据 _data.type 判断行为
    if (_data.type == "shooter") {
        CCLOG("Plant %s shoots a pea!", _data.name.c_str());
        // TODO: 生成子弹
    }
    else if (_data.type == "producer") {
        CCLOG("Plant %s produces sun!", _data.name.c_str());
        // TODO: 生成阳光
    }
}

void Plant::die() {
    CCLOG("Plant %s died!", _data.name.c_str());
    Unit::die();
}