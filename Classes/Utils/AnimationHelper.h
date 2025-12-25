// 动画辅助工具类
// 用于从动画配置创建 Cocos2d-x Animation 对象
// 2025.12.2 by BillyDu
#ifndef __ANIMATION_HELPER_H__
#define __ANIMATION_HELPER_H__

#include "cocos2d.h"
#include "../Entities/GameDataStructures.h"

USING_NS_CC;

class AnimationHelper {
public:
    // 从动画配置创建 Animation 对象
    static Animation* createAnimationFromConfig(const AnimationConfig& config);
    
    // 从动画配置创建并运行动画（返回 Action 对象，可能是 Animate 或 RepeatForever）
    static Action* createAnimateFromConfig(const AnimationConfig& config);
    
    // 从动画配置创建有限时间的动画动作（用于序列化，不包含 RepeatForever）
    static FiniteTimeAction* createFiniteAnimateFromConfig(const AnimationConfig& config);
};

#endif // __ANIMATION_HELPER_H__

