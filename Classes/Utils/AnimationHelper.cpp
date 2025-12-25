// 实现动画辅助工具类
// 2025.12.2 by BillyDu
#include "AnimationHelper.h"
#include "cocos2d.h"

USING_NS_CC;

Animation* AnimationHelper::createAnimationFromConfig(const AnimationConfig& config) {
    if (config.frameFormat.empty() || config.frameCount <= 0) {
        CCLOG("[Err] Invalid animation config: frameFormat=%s, frameCount=%d", 
              config.frameFormat.c_str(), config.frameCount);
        return nullptr;
    }
    
    Vector<SpriteFrame*> frames;
    frames.reserve(config.frameCount);
    
    // 根据 frameFormat 和 frameCount 加载所有帧
    for (int i = 1; i <= config.frameCount; ++i) {
        // 替换 %d 为帧序号
        char framePath[256];
        snprintf(framePath, sizeof(framePath), config.frameFormat.c_str(), i);
        
        // 尝试从 SpriteFrameCache 获取，如果不存在则创建
        SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(framePath);
        if (!frame) {
            // 如果缓存中没有，直接从文件加载
            Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(framePath);
            if (texture) {
                Rect rect = Rect::ZERO;
                rect.size = texture->getContentSize();
                frame = SpriteFrame::createWithTexture(texture, rect);
                CCLOG("[Debug] Loaded frame %d: %s (size: %.0f x %.0f)", 
                      i, framePath, rect.size.width, rect.size.height);
            } else {
                CCLOG("[Err] Failed to load texture for frame: %s", framePath);
            }
        } else {
            CCLOG("[Debug] Frame %d loaded from cache: %s", i, framePath);
        }
        
        if (frame) {
            frames.pushBack(frame);
        } else {
            CCLOG("[Warn] Failed to create frame: %s", framePath);
        }
    }
    
    if (frames.empty()) {
        CCLOG("[Err] No frames loaded for animation: %s", config.frameFormat.c_str());
        return nullptr;
    }
    
    // 创建 Animation 对象
    Animation* animation = Animation::createWithSpriteFrames(frames, config.frameDelay);
    return animation;
}

Action* AnimationHelper::createAnimateFromConfig(const AnimationConfig& config) {
    Animation* animation = createAnimationFromConfig(config);
    if (!animation) {
        return nullptr;
    }
    
    Animate* animate = Animate::create(animation);
    
    // 根据 loopCount 设置循环
    if (config.loopCount == -1) {
        // 无限循环
        return RepeatForever::create(animate);
    } else if (config.loopCount > 1) {
        // 循环指定次数
        return Repeat::create(animate, config.loopCount);
    }
    // loopCount == 1 表示播放一次，不需要额外处理
    
    return animate;
}

FiniteTimeAction* AnimationHelper::createFiniteAnimateFromConfig(const AnimationConfig& config) {
    Animation* animation = createAnimationFromConfig(config);
    if (!animation) {
        return nullptr;
    }
    
    Animate* animate = Animate::create(animation);
    
    // 对于有限时间的动画，根据 loopCount 设置循环
    if (config.loopCount > 1) {
        // 循环指定次数
        return Repeat::create(animate, config.loopCount);
    }
    // loopCount == 1 或 <= 0 表示播放一次，直接返回 Animate
    
    return animate;
}

