// 实现子弹类的逻辑
// 2025.12.12 by BillyDu
#include "Bullet.h"

USING_NS_CC;

Bullet* Bullet::create(const BulletData& data) {
    Bullet* ret = new (std::nothrow) Bullet();
    if (ret && ret->init()) {
        ret->setBulletData(data);
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool Bullet::init() {
    if (!Unit::init()) return false;
    _type = UnitType::BULLET;
    _active = true;
    return true;
}

void Bullet::setBulletData(const BulletData& data) {
    _data = data;

    // 加载贴图 (如果没有可以用绿点代替)
    if (FileUtils::getInstance()->isFileExist(_data.texturePath)) {
        this->setTexture(_data.texturePath);
    }
    else {
        // 兜底：画一个小圆点
        auto drawNode = DrawNode::create();
        drawNode->drawDot(Vec2::ZERO, 10, Color4F::RED);
        this->addChild(drawNode);
    }
}

void Bullet::updateLogic(float dt) {
    if (!_active) return;

    // 子弹简单地向右飞
    float moveDist = _data.speed * dt;
    this->setPositionX(this->getPositionX() + moveDist);

    // 越界销毁 (假设屏幕宽 1280)
    if (this->getPositionX() > 1300) {
        this->removeFromParent();
        _active = false;
    }
}