// Bullet class implementation
// 2025.12.12 by BillyDu 12.21 by Zhao (add animation support)
#include "Bullet.h"
#include "../Utils/AnimationHelper.h"

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

    // If has animation config, play animation
    if (_data.hasAnimation && !_data.animationConfig.frameFormat.empty()) {
        // Create and play animation
        Action* animate = AnimationHelper::createAnimateFromConfig(_data.animationConfig);
        if (animate) {
            // Set initial texture (first frame)
            std::string firstFramePath = _data.animationConfig.defaultTexture;
            if (firstFramePath.empty()) {
                // Auto-generate first frame path
                char defaultPath[256];
                snprintf(defaultPath, sizeof(defaultPath), _data.animationConfig.frameFormat.c_str(), 1);
                firstFramePath = defaultPath;
            }
            if (!firstFramePath.empty() && FileUtils::getInstance()->isFileExist(firstFramePath)) {
                this->setTexture(firstFramePath);
            }
            
            // Run animation
            this->runAction(animate);
            CCLOG("[Info] Bullet animation started: %s", _data.animationConfig.frameFormat.c_str());
        } else {
            // Fallback to static texture if animation creation failed
            if (FileUtils::getInstance()->isFileExist(_data.texturePath)) {
                this->setTexture(_data.texturePath);
            } else {
                // Fallback: draw a small circle
                auto drawNode = DrawNode::create();
                drawNode->drawDot(Vec2::ZERO, 10, Color4F::RED);
                this->addChild(drawNode);
            }
        }
    } else {
        // Use static texture
        if (FileUtils::getInstance()->isFileExist(_data.texturePath)) {
            this->setTexture(_data.texturePath);
        } else {
            // Fallback: draw a small circle
            auto drawNode = DrawNode::create();
            drawNode->drawDot(Vec2::ZERO, 10, Color4F::RED);
            this->addChild(drawNode);
        }
    }
}

void Bullet::updateLogic(float dt) {
    if (!_active) return;

    // Bullet moves to the right
    float moveDist = _data.speed * dt;
    this->setPositionX(this->getPositionX() + moveDist);

    // Out of bounds check (screen width is 1280)
    if (this->getPositionX() > 1300) {
        this->removeFromParent();
        _active = false;
    }
}
