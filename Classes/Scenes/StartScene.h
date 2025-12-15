// 开始菜单场景
// 2025.12.15 by BillyDu
#ifndef __START_SCENE_H__
#define __START_SCENE_H__

#include "cocos2d.h"

class StartScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    
    CREATE_FUNC(StartScene);
    
private:
    // 按钮回调函数
    void onStartButtonClicked(cocos2d::Ref* sender);
    void onSettingsButtonClicked(cocos2d::Ref* sender);
    void onExitButtonClicked(cocos2d::Ref* sender);
    
    // UI 组件
    void createBackground();
    void createTitle();
    void createButtons();
    
    // 成员变量
    cocos2d::Size _visibleSize;
    cocos2d::Vec2 _origin;
};

#endif // __START_SCENE_H__