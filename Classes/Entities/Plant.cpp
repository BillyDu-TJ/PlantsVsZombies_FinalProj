// 实现植物类
// 2025.12.2 by BillyDu
#include "Plant.h"
#include "../Utils/AnimationHelper.h"

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
    _currentAnimation = "";
    return true;
}

void Plant::setPlantData(const PlantData& data) {
    _data = data;
    this->setHp(data.hp);

    // 优先使用动画，如果没有动画配置则使用静态纹理
    if (!_data.animations.empty() && !_data.defaultAnimation.empty()) {
        // 有动画配置，播放默认动画
        CCLOG("[Info] Plant %s: Loading animation '%s'", _data.name.c_str(), _data.defaultAnimation.c_str());
        playDefaultAnimation();
    } else if (!_data.texturePath.empty()) {
        // 没有动画配置，使用静态纹理
        CCLOG("[Info] Plant %s: Using static texture: %s", _data.name.c_str(), _data.texturePath.c_str());
        this->setTexture(_data.texturePath);
    } else {
        CCLOG("[Warn] Plant %s: No texture or animation configured", _data.name.c_str());
    }
}

void Plant::playAnimation(const std::string& animName) {
    // 如果已经在播放相同的动画，不重复播放
    if (_currentAnimation == animName) {
        return;
    }
    
    // 查找动画配置
    auto it = _data.animations.find(animName);
    if (it == _data.animations.end()) {
        CCLOG("[Warn] Animation '%s' not found for plant %s", animName.c_str(), _data.name.c_str());
        return;
    }
    
    const AnimationConfig& animConfig = it->second;
    
    // 停止当前所有动画
    this->stopAllActions();
    
    // 创建并运行动画
    Action* animate = AnimationHelper::createAnimateFromConfig(animConfig);
    if (animate) {
        // 设置初始纹理（第一帧），确保动画开始前显示正确的帧
        std::string firstFramePath = animConfig.defaultTexture;
        if (firstFramePath.empty()) {
            // 如果没有指定defaultTexture，自动生成第一帧路径
            char defaultPath[256];
            snprintf(defaultPath, sizeof(defaultPath), animConfig.frameFormat.c_str(), 1);
            firstFramePath = defaultPath;
        }
        if (!firstFramePath.empty()) {
            this->setTexture(firstFramePath);
        }
        
        // 根据动画类型处理完成后的行为
        if (animConfig.loopCount == 1 && !animConfig.onComplete.empty()) {
            // 播放一次的动画，需要处理完成后的行为
            // 使用有限时间动作版本，因为需要放入 Sequence
            FiniteTimeAction* finiteAnimate = AnimationHelper::createFiniteAnimateFromConfig(animConfig);
            if (!finiteAnimate) {
                CCLOG("[Err] Failed to create finite animate for '%s'", animName.c_str());
                return;
            }
            
            if (animConfig.onComplete == "idle") {
                // 播放完成后切换到待机动画
                auto idleIt = _data.animations.find("idle");
                if (idleIt != _data.animations.end()) {
                    // 按值捕获动画配置，避免引用问题
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
                // 播放完成后移除
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
        
        // 默认情况：直接运行动画
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
        // 如果没有默认动画，使用第一个动画
        playAnimation(_data.animations.begin()->first);
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
        
        // 如果有射击动画，播放它
        auto shootIt = _data.animations.find("shoot");
        if (shootIt != _data.animations.end()) {
            playAnimation("shoot");
        }

        if (_onShootCallback) {
            // 子弹发射位置：植物中心向右一点，向上一点(嘴巴位置)
            Vec2 spawnPos = this->getPosition() + Vec2(20, 10);//
            _onShootCallback(spawnPos, _data.attack);
        } else {
            CCLOG("[Err] _onShootCallback is NULL! Did you set it in GameScene?");
        }
    }
    else if (_data.type == "producer") {
        // 如果有生产动画，播放它
        auto produceIt = _data.animations.find("produce");
        if (produceIt != _data.animations.end()) {
            playAnimation("produce");
        }
        // ...
    } 
    else {
        CCLOG("[Err] Plant type mismatch! Expected 'shooter', got '%s'", _data.type.c_str());
    }
}

void Plant::die() {
    CCLOG("Plant %s died!", _data.name.c_str());
    
    // 如果有死亡动画，播放它
    auto deadIt = _data.animations.find("dead");
    if (deadIt != _data.animations.end()) {
        playAnimation("dead");
    } else {
        // 没有死亡动画，直接调用父类的die
        Unit::die();
    }
}

void Plant::setOnShootCallback(const std::function<void(cocos2d::Vec2, int)>& callback) {
    _onShootCallback = callback;
}
