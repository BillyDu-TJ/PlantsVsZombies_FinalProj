// 定义了游戏中植物和僵尸的基础数据结构
// 2025.11.27 by BillyDu
#ifndef __GAME_DATA_STRUCTURES_H__
#define __GAME_DATA_STRUCTURES_H__

#include <string>

// 植物的基础属性数据
struct PlantData {
    std::string name;
    std::string type;     // "shooter" or "producer"
    int hp = 0;
    int cost = 0;
    float cooldown = 0.0f;
    int attack = 0;
    float attackSpeed = 0.0f; // 攻击间隔
    std::string texturePath;
};

// 僵尸的基础属性数据
struct ZombieData {
    std::string name;
    int hp = 0;
    float speed = 0.0f;
    int attack = 0;
    float attackSpeed = 0.0f; // 攻击间隔
	std::string texturePath;
};

#endif // __GAME_DATA_STRUCTURES_H__