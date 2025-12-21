// Zombie class definition, inherits from Unit class
// 2025.12.2 by BillyDu 12.21 modify by Zhao
#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__

#include <string>
#include "Unit.h"
#include "GameDataStructures.h"

class Zombie : public Unit {
public:
    static Zombie* createWithData(const ZombieData& data);
    virtual bool init() override;

    virtual void updateLogic(float dt) override;
    virtual void die() override;

    void setZombieData(const ZombieData& data);

    // Check if can attack (cooldown ready)
    bool canAttack() const;

    // Reset attack timer
    void resetAttackTimer();

    // Get damage
    int getDamage() const { return _data.damage; }
    
    // Apply slow effect (speed multiplier, e.g., 0.5 for 50% speed)
    void applySlowEffect(float speedMultiplier);
    
    // Animation related methods
    void playAnimation(const std::string& animName);  // Play specified animation
    void playDefaultAnimation();  // Play default animation

private:
    ZombieData _data;
	float _attackTimer = 0.0f;
    std::string _currentAnimation;  // Current playing animation name
    float _speedMultiplier = 1.0f;  // Current speed multiplier (1.0 = normal, 0.5 = slowed)
};

#endif // __ZOMBIE_H__
