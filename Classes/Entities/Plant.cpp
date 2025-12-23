// Plant class implementation
// 2025.12.2 by BillyDu
#include "Plant.h"
#include "../Utils/AnimationHelper.h"

USING_NS_CC;

Plant* Plant::createWithData(const PlantData& data) {
    Plant* pRet = new(std::nothrow) Plant();
    if (pRet && pRet->init()) {
        pRet->setPlantData(data);
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
    _currentAnimation = "";
    return true;
}

void Plant::setPlantData(const PlantData& data) {
    _data = data;
    this->setHp(data.hp);

    // Use animation if available, otherwise use static texture
    if (!_data.animations.empty() && !_data.defaultAnimation.empty()) {
        // Has animation config, play default animation
        CCLOG("[Info] Plant %s: Loading animation '%s'", _data.name.c_str(), _data.defaultAnimation.c_str());
        playDefaultAnimation();
    } else if (!_data.texturePath.empty()) {
        // No animation config, use static texture
        CCLOG("[Info] Plant %s: Using static texture: %s", _data.name.c_str(), _data.texturePath.c_str());
        this->setTexture(_data.texturePath);
    } else {
        CCLOG("[Warn] Plant %s: No texture or animation configured", _data.name.c_str());
    }
}

void Plant::playAnimation(const std::string& animName) {
    // Don't replay if already playing the same animation
    if (_currentAnimation == animName) {
        return;
    }
    
    // Find animation config
    auto it = _data.animations.find(animName);
    if (it == _data.animations.end()) {
        CCLOG("[Warn] Animation '%s' not found for plant %s", animName.c_str(), _data.name.c_str());
        return;
    }
    
    const AnimationConfig& animConfig = it->second;
    
    // Stop all current actions
    this->stopAllActions();
    
    // Create and run animation
    Action* animate = AnimationHelper::createAnimateFromConfig(animConfig);
    if (animate) {
        // Set initial texture (first frame) to ensure correct frame before animation starts
        std::string firstFramePath = animConfig.defaultTexture;
        if (firstFramePath.empty()) {
            // If no defaultTexture specified, auto-generate first frame path
            char defaultPath[256];
            snprintf(defaultPath, sizeof(defaultPath), animConfig.frameFormat.c_str(), 1);
            firstFramePath = defaultPath;
        }
        if (!firstFramePath.empty()) {
            this->setTexture(firstFramePath);
        }
        
        // Handle completion behavior based on animation type
        if (animConfig.loopCount == 1 && !animConfig.onComplete.empty()) {
            // One-time animation, need to handle completion behavior
            // Use finite time action version because it needs to be in Sequence
            FiniteTimeAction* finiteAnimate = AnimationHelper::createFiniteAnimateFromConfig(animConfig);
            if (!finiteAnimate) {
                CCLOG("[Err] Failed to create finite animate for '%s'", animName.c_str());
                return;
            }
            
            if (animConfig.onComplete == "idle") {
                // Switch to idle animation after completion
                auto idleIt = _data.animations.find("idle");
                if (idleIt != _data.animations.end()) {
                    // Capture animation config by value to avoid reference issues
                    AnimationConfig idleConfig = idleIt->second;
                    this->runAction(Sequence::create(
                        finiteAnimate,
                        CallFunc::create([this, idleConfig]() {
                            Action* idleAnimate = AnimationHelper::createAnimateFromConfig(idleConfig);
                            if (idleAnimate) {
                                this->runAction(idleAnimate);
                                this->_currentAnimation = "idle";
                            }
                        }),
                        nullptr
                    ));
                    _currentAnimation = animName;
                    return;
                }
            } else if (animConfig.onComplete == "remove") {
                // Remove after completion
                this->runAction(Sequence::create(
                    finiteAnimate,
                    CallFunc::create([this]() {
                        this->removeFromParent();
                    }),
                    nullptr
                ));
                _currentAnimation = animName;
                return;
            }
        }
        
        // Default case: run animation directly
        this->runAction(animate);
        _currentAnimation = animName;
    } else {
        CCLOG("[Err] Failed to create animation '%s' for plant %s", animName.c_str(), _data.name.c_str());
    }
}

void Plant::playDefaultAnimation() {
    if (!_data.defaultAnimation.empty()) {
        playAnimation(_data.defaultAnimation);
    } else if (!_data.animations.empty()) {
        // If no default animation, use first animation
        playAnimation(_data.animations.begin()->first);
    }
}

void Plant::updateLogic(float dt) {
    Unit::updateLogic(dt);

    // Simple cooldown logic
    if (_data.attackSpeed > 0) {
        _timer += dt;

        if (_timer >= _data.attackSpeed) {
            // Note: using attackSpeed as attack/production interval
            CCLOG("[Debug] Plant Timer Reached! (Timer: %.2f, Speed: %.2f)", _timer, _data.attackSpeed);
            _timer = 0;
            triggerSkill();
        }
    } else {
        // Debug check: if attackSpeed is 0 or negative, report error
        // Limit print frequency to prevent spam
        static bool hasWarned = false;
        if (!hasWarned) {
            CCLOG("[Err] Plant %s has 0 or negative attackSpeed: %f", _data.name.c_str(), _data.attackSpeed);
            hasWarned = true;
        }
    }
}

void Plant::triggerSkill() {
    CCLOG("[Debug] triggerSkill called for: %s (Type: %s)", _data.name.c_str(), _data.type.c_str());

    // Can determine behavior based on _data.type here
    if (_data.type == "shooter") {
        CCLOG("[Info] Plant %s shoots!", _data.name.c_str());
        
        // If has shoot animation, play it (will return to idle automatically via onComplete)
        auto shootIt = _data.animations.find("shoot");
        if (shootIt != _data.animations.end()) {
            playAnimation("shoot");
        }

        if (_onShootCallback) {
            // Bullet spawn position: plant center slightly right and up (mouth position)
            Vec2 spawnPos = this->getPosition() + Vec2(20, 10);
            _onShootCallback(spawnPos, _data.attack);
        } else {
            CCLOG("[Err] _onShootCallback is NULL! Did you set it in GameScene?");
        }
    }
    else if (_data.type == "producer") {
        // 生产类植物（向日葵、阳光菇等）
        CCLOG("[Info] Plant %s produces!", _data.name.c_str());

        // 如果有生产动画，播放一次
        auto produceIt = _data.animations.find("produce");
        if (produceIt != _data.animations.end()) {
            playAnimation("produce");
        }

        // 调用回调，由 GameScene 决定具体产生什么（这里是生成阳光）
        if (_onShootCallback) {
            // 生成位置：以植物中心为基准，稍微偏上一点
            cocos2d::Vec2 spawnPos = this->getPosition() + cocos2d::Vec2(0, 20);
            _onShootCallback(spawnPos, 0);
        } else {
            CCLOG("[Err] _onShootCallback for producer is NULL! Did you set it in GameScene?");
        }
    } 
    else if (_data.type == "defensive") {
        // 防御类植物（如 Spikeweed 等）
        CCLOG("[Info] Defensive plant %s triggers skill!", _data.name.c_str());

        // Spikeweed 有攻击动画
        auto atkIt = _data.animations.find("attack");
        if (atkIt != _data.animations.end()) {
            playAnimation("attack");
        }

        if (_onShootCallback) {
            // 使用植物当前位置作为作用中心
            cocos2d::Vec2 centerPos = this->getPosition();
            _onShootCallback(centerPos, _data.attack);
        } else {
            CCLOG("[Debug] Defensive plant %s has no callback set.", _data.name.c_str());
        }
    }
    else {
        CCLOG("[Err] Plant type mismatch! Unsupported type '%s' for plant %s", _data.type.c_str(), _data.name.c_str());
    }
}

void Plant::die() {
    CCLOG("Plant %s died!", _data.name.c_str());
    
    // If has death animation, play it
    auto deadIt = _data.animations.find("dead");
    if (deadIt != _data.animations.end()) {
        playAnimation("dead");
    } else {
        // No death animation, directly call parent's die
        Unit::die();
    }
}

void Plant::setOnShootCallback(const std::function<void(cocos2d::Vec2, int)>& callback) {
    _onShootCallback = callback;
}

void Plant::setHasTargetCallback(const std::function<bool()>& callback) {
    _hasTargetCallback = callback;
}

