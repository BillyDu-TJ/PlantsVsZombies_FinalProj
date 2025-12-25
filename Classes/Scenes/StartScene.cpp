// 开始菜单场景实现
// 2025.12.15 by BillyDu
#include "StartScene.h"
#include "../Managers/SceneManager.h"
#include "../Managers/AudioManager.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

Scene* StartScene::createScene() {
    return StartScene::create();
}

bool StartScene::init() {
    if (!Scene::init()) {
        return false;
    }
    
    _visibleSize = Director::getInstance()->getVisibleSize();
    _origin = Director::getInstance()->getVisibleOrigin();
    
    createBackground();
    createTitle();
    createButtons();
    
    return true;
}

void StartScene::createBackground() {
    auto bg = Sprite::create("bg/menu.png");
    bg->setPosition(_visibleSize.width/2 + _origin.x, _visibleSize.height/2 + _origin.y);
    this->addChild(bg, -1);
}

void StartScene::createTitle() {
    // 游戏标题
    auto title = Label::createWithTTF("Plants vs Zombies", "fonts/Marker Felt.ttf", 64);
    title->setPosition(_visibleSize.width/2 + _origin.x, _visibleSize.height * 0.75f + _origin.y);
    title->setColor(Color3B::YELLOW);
    this->addChild(title, 1);
    
    // 添加标题动画效果
    auto scaleUp = ScaleTo::create(1.0f, 1.1f);
    auto scaleDown = ScaleTo::create(1.0f, 1.0f);
    auto sequence = Sequence::create(scaleUp, scaleDown, nullptr);
    auto repeatAction = RepeatForever::create(sequence);
    title->runAction(repeatAction);
}

void StartScene::createButtons() {
    float centerX = _visibleSize.width/2 + _origin.x;
    float startY = _visibleSize.height * 0.5f + _origin.y;
    float buttonSpacing = 80.0f;
    
    // 开始游戏按钮
    auto startButton = ui::Button::create();
    startButton->setTitleText("Start Game");
    startButton->setTitleFontName("fonts/Marker Felt.ttf");
    startButton->setTitleFontSize(32);
    startButton->setTitleColor(Color3B::WHITE);
    startButton->setColor(Color3B(0, 100, 0));
    startButton->setPosition(Vec2(centerX, startY));
    startButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onStartButtonClicked(sender);
        }
    });
    this->addChild(startButton, 1);
    
    // 设置按钮
    auto settingsButton = ui::Button::create();
    settingsButton->setTitleText("Settings");
    settingsButton->setTitleFontName("fonts/Marker Felt.ttf");
    settingsButton->setTitleFontSize(32);
    settingsButton->setTitleColor(Color3B::WHITE);
    settingsButton->setColor(Color3B(100, 100, 0));
    settingsButton->setPosition(Vec2(centerX, startY - buttonSpacing));
    settingsButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onSettingsButtonClicked(sender);
        }
    });
    this->addChild(settingsButton, 1);
    
    // 退出按钮
    auto exitButton = ui::Button::create();
    exitButton->setTitleText("Exit");
    exitButton->setTitleFontName("fonts/Marker Felt.ttf");
    exitButton->setTitleFontSize(32);
    exitButton->setTitleColor(Color3B::WHITE);
    exitButton->setColor(Color3B(100, 0, 0));
    exitButton->setPosition(Vec2(centerX, startY - buttonSpacing * 2));
    exitButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onExitButtonClicked(sender);
        }
    });
    this->addChild(exitButton, 1);
}

void StartScene::onStartButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Start button clicked");
    AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND); // 临时用种植音效
    SceneManager::getInstance().gotoMapSelectScene();
}

void StartScene::onSettingsButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Settings button clicked");
    // TODO: 实现设置界面
    // 这里可以创建一个弹窗或跳转到设置场景
    AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);
    
    // 简单的音量控制示例
    auto& audioMgr = AudioManager::getInstance();
    float currentVolume = audioMgr.getMusicVolume();
    audioMgr.setMusicVolume(currentVolume > 0.5f ? 0.3f : 1.0f);
}

void StartScene::onExitButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Exit button clicked");
    Director::getInstance()->end();
}