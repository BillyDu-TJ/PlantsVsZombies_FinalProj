// Zombie class implementation
// 2025.12.2 by BillyDu
// 2025.12.21 by Zhao (add slow effect support)
#include "Zombie.h"
#include "../Consts.h"
#include "../Utils/AnimationHelper.h"

USING_NS_CC;

Zombie* Zombie::createWithData(const ZombieData& data) {
    Zombie* pRet = new(std::nothrow) Zombie();
    if (pRet && pRet->init()) {
        pRet->setZombieData(data);
        pRet->autorelease();
        return pRet;
    }
    else {
        delete pRet;
        return nullptr;
    }
}

bool Zombie::init() {
    if (!Unit::init()) return false;

    _type = UnitType::ZOMBIE;
    _state = UnitState::WALK;
    _attackTimer = 0.0f;
    _lifeTimer = 0.0f;  // Initialize life timer
    _currentAnimation = "";

    return true;
}

void Zombie::setZombieData(const ZombieData& data) {
    _data = data;

    // 1. Set HP
    this->setHp(data.hp);

    // 2. Use animation if available, otherwise use static texture
    if (!_data.animations.empty() && !_data.defaultAnimation.empty()) {
        // Has animation config, play default animation
        CCLOG("[Info] Zombie %s: Loading animation '%s'", _data.name.c_str(), _data.defaultAnimation.c_str());
        playDefaultAnimation();
    } else if (!_data.texturePath.empty()) {
        // No animation config, use static texture
        CCLOG("[Info] Zombie %s: Using static texture: %s", _data.name.c_str(), _data.texturePath.c_str());
        this->setTexture(_data.texturePath);
    } else {
        // Fallback
        this->setTextureRect(Rect(0, 0, 60, 90));
        this->setColor(Color3B::RED);
        CCLOG("[Warn] Zombie %s used fallback color block.", _data.name.c_str());
    }
}

void Zombie::playAnimation(const std::string& animName) {
    // Don't replay if already playing the same animation
    if (_currentAnimation == animName) {
        return;
    }
    
    // Find animation config
    auto it = _data.animations.find(animName);
    if (it == _data.animations.end()) {
        CCLOG("[Warn] Animation '%s' not found for zombie %s", animName.c_str(), _data.name.c_str());
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
            FiniteTimeAction* finiteAnimate = AnimationHelper::createFiniteAnimateFromConfig(animConfig);
            if (!finiteAnimate) {
                CCLOG("[Err] Failed to create finite animate for '%s'", animName.c_str());
                return;
            }
            
            if (animConfig.onComplete == "remove") {
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
        CCLOG("[Err] Failed to create animation '%s' for zombie %s", animName.c_str(), _data.name.c_str());
    }
}

void Zombie::playDefaultAnimation() {
    if (!_data.defaultAnimation.empty()) {
        playAnimation(_data.defaultAnimation);
    } else if (!_data.animations.empty()) {
        // If no default animation, use first animation
        playAnimation(_data.animations.begin()->first);
    }
}

void Zombie::checkPhaseTransition() {
    if (_maxHp <= 0) return;
    
    int currentHp = getHp();
    float hpPercent = (float)currentHp / (float)_maxHp * 100.0f;
    
    // Boss1: 30% threshold (move1/eat1 -> move2/eat2)
    if (_data.name == "Boss1" && !_isPhase2) {
        if (hpPercent <= 30.0f) {
            _isPhase2 = true;
            CCLOG("[Info] Boss1 entered Phase 2! HP: %.1f%% (%d/%d)", hpPercent, currentHp, _maxHp);
            
            // Switch animations based on current state
            if (_state == UnitState::WALK) {
                if (_currentAnimation == "move1") {
                    playAnimation("move2");
                }
            }
            else if (_state == UnitState::ATTACK) {
                if (_currentAnimation == "eat1") {
                    playAnimation("eat2");
                }
            }
        }
    }
    // Boss2: Multi-phase transitions (75% -> move2, 40% -> move3, 20% -> move4)
    else if (_data.name == "Boss2") {
        int targetPhase = 1;
        if (hpPercent <= 20.0f) {
            targetPhase = 4;
        }
        else if (hpPercent <= 40.0f) {
            targetPhase = 3;
        }
        else if (hpPercent <= 75.0f) {
            targetPhase = 2;
        }
        
        if (targetPhase != _currentPhase) {
            int oldPhase = _currentPhase;
            _currentPhase = targetPhase;
            std::string animName = "move" + std::to_string(targetPhase);
            CCLOG("[Info] Boss2 phase transition: %d -> %d (HP: %.1f%%)", oldPhase, targetPhase, hpPercent);
            playAnimation(animName);
        }
    }
}

void Zombie::updateLogic(float dt) {
    Unit::updateLogic(dt);

	_attackTimer += dt;
    _lifeTimer += dt;  // Update life timer for walk1->walk2 transition
    
    // Check for phase transition (for boss zombies)
    checkPhaseTransition();

    // Boss2 (snow sled) always moves forward, never stops
    if (_data.name == "Boss2") {
        // Always use move animation based on current phase
        std::string targetAnim = "move" + std::to_string(_currentPhase);
        if (_currentAnimation != targetAnim) {
            playAnimation(targetAnim);
        }
        
        // Always move forward (crushing type, never stops)
        float moveDist = _data.speed * _speedMultiplier * dt;
        this->setPositionX(this->getPositionX() - moveDist);
        
        // Simple boundary check
        if (this->getPositionX() < -50) {
            this->removeFromParent();
            CCLOG("[Info] Boss2 reached the house! Game Over?");
			// TODO: Trigger game over logic
        }
        return; // Boss2 doesn't use normal attack logic
    }

    if (_state == UnitState::WALK) {
        // For boss1, use move1/move2 instead of walk
        if (_data.name == "Boss1") {
            std::string targetAnim = _isPhase2 ? "move2" : "move1";
            if (_currentAnimation != targetAnim) {
                playAnimation(targetAnim);
            }
        }
        else {
            // 普通僵尸：如果有 walk1/walk2，则先走10秒walk1，然后自动切换为walk2（游泳）
            auto itWalk1 = _data.animations.find("walk1");
            auto itWalk2 = _data.animations.find("walk2");
            if (itWalk1 != _data.animations.end() && itWalk2 != _data.animations.end()) {
                // 游泳僵尸：前10秒用walk1，10秒后自动切换为walk2
                std::string targetAnim = (_lifeTimer >= 15.0f) ? "walk2" : "walk1";
                if (_currentAnimation != targetAnim) {
                    playAnimation(targetAnim);
                }
            } else {
                // 普通只有一个 "walk" 动画的僵尸
                if (_currentAnimation != "walk") {
                    playAnimation("walk");
                }
            }
        }
        
        // Move left (apply speed multiplier for slow effects)
        float moveDist = _data.speed * _speedMultiplier * dt;
        this->setPositionX(this->getPositionX() - moveDist);

        // Simple boundary check
        if (this->getPositionX() < -50) {
            this->removeFromParent();
            CCLOG("[Info] Zombie reached the house! Game Over?");
			// TODO: Trigger game over logic
        }
    }
    else if (_state == UnitState::ATTACK) {
        // For boss1, use eat1/eat2 instead of eat
        if (_data.name == "Boss1") {
            std::string targetAnim = _isPhase2 ? "eat2" : "eat1";
            if (_currentAnimation != targetAnim) {
                playAnimation(targetAnim);
            }
        }
        else {
            // Normal zombies use "eat"
            if (_currentAnimation != "eat") {
                playAnimation("eat");
            }
        }
		// Attack logic is handled externally, here we only maintain state and animation
    }
}

void Zombie::die() {
    CCLOG("Zombie %s died!", _data.name.c_str());
    
    // For boss1 and boss2, use "die" animation
    std::string deathAnim = ((_data.name == "Boss1") || (_data.name == "Boss2")) ? "die" : "dead";
    
    // If has death animation, play it
    auto deadIt = _data.animations.find(deathAnim);
    if (deadIt != _data.animations.end()) {
        playAnimation(deathAnim);
    } else {
        // No death animation, directly call parent's die
        Unit::die();
    }
}

bool Zombie::canAttack() const {
    // If current attack timer exceeds the configured attack interval, can attack
    return _attackTimer >= _data.attackInterval;
}

void Zombie::resetAttackTimer() {
    _attackTimer = 0.0f; // Reset, start cooldown
}

void Zombie::applySlowEffect(float speedMultiplier) {
    // Apply slow effect (lower value = slower)
    // Always apply the slow effect, even if it's the same or higher
    // This ensures the effect is refreshed and visible
    _speedMultiplier = speedMultiplier;
    CCLOG("[Info] Zombie %s slowed to %.1f%% speed (original speed: %.1f, current speed: %.1f)", 
          _data.name.c_str(), 
          speedMultiplier * 100.0f,
          _data.speed,
          _data.speed * _speedMultiplier);
}
