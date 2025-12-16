// 定义游戏中植物和僵尸的基本数据结构
// 2025.11.27 by BillyDu
#ifndef __GAME_DATA_STRUCTURES_H__
#define __GAME_DATA_STRUCTURES_H__

#include <string>
#include <unordered_map>

// 动画配置结构
struct AnimationConfig {
    std::string frameFormat;  // 帧路径格式，如 "plants/peashooter/%d.png"
    int frameCount = 0;        // 总帧数
    float frameDelay = 0.1f;   // 每帧延迟时间（秒）
    int loopCount = -1;        // 循环次数：-1表示无限循环，1表示播放一次
    std::string defaultTexture; // 默认纹理（通常是第一帧）
    std::string onComplete;     // 动画完成后的行为："idle", "remove", "none"
};

// 植物的基本数据结构
struct PlantData {
    std::string name;
    std::string type;     // "shooter" or "producer"
    int hp = 0;
    int cost = 0;
    float cooldown = 0.0f;
    int attack = 0;
    float attackSpeed = 0.0f; // 攻击速度
    std::string texturePath;
    
    // 动画配置：动画名称 -> 动画配置
    std::unordered_map<std::string, AnimationConfig> animations;
    std::string defaultAnimation; // 默认动画名称，如 "idle"
};

// 僵尸的基本数据结构
struct ZombieData {
    std::string name;
    int hp = 0;
    int damage = 0;
    float speed = 0.0f;          // 移动速度
    float attackInterval = 1.0f; // 攻击间隔
    std::string texturePath;
};

// 子弹数据
struct BulletData {
    std::string name;
    int damage = 20;
    float speed = 300.0f; // 移动速度
    std::string texturePath;
    // 以后可以加减速、伤害范围等
};

#endif // __GAME_DATA_STRUCTURES_H__
