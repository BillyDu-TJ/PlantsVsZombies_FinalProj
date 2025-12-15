// ”Œœ∑ ß∞‹≥°æ∞
// 2025.12.15 by BillyDu
#ifndef __GAME_OVER_SCENE_H__
#define __GAME_OVER_SCENE_H__

#include "cocos2d.h"

class GameOverScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    
    CREATE_FUNC(GameOverScene);
    
private:
    void onRestartButtonClicked(cocos2d::Ref* sender);
    void onMainMenuButtonClicked(cocos2d::Ref* sender);
    
    void createBackground();
    void createUI();
    
    cocos2d::Size _visibleSize;
    cocos2d::Vec2 _origin;
};

#endif // __GAME_OVER_SCENE_H__