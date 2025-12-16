// 游戏胜利场景实现
// 2025.12.15 by BillyDu
#include "VictoryScene.h"
#include "../Managers/SceneManager.h"
#include "../Managers/AudioManager.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

Scene* VictoryScene::createScene() {
    return VictoryScene::create();
}

bool VictoryScene::init() {
    if (!Scene::init()) {
        return false;
    }
    
    _visibleSize = Director::getInstance()->getVisibleSize();
    _origin = Director::getInstance()->getVisibleOrigin();
    
    createBackground();
    createUI();
    
    return true;
}

void VictoryScene::createBackground() {
    // 金黄色背景表示胜利
    auto bg = LayerGradient::create(Color4B(255, 215, 0, 255), Color4B(255, 140, 0, 255));
    this->addChild(bg, -1);
}

void VictoryScene::createUI() {
    float centerX = _visibleSize.width/2 + _origin.x;
    float centerY = _visibleSize.height/2 + _origin.y;
    
    // "Victory!" 标题
    auto victoryLabel = Label::createWithTTF("VICTORY!", "fonts/Marker Felt.ttf", 72);
    victoryLabel->setPosition(centerX, centerY + 100);
    victoryLabel->setColor(Color3B::YELLOW);
    this->addChild(victoryLabel, 1);
    
    // 添加胜利动画
    auto scaleUp = ScaleTo::create(0.5f, 1.2f);
    auto scaleDown = ScaleTo::create(0.5f, 1.0f);
    auto sequence = Sequence::create(scaleUp, scaleDown, nullptr);
    auto repeatAction = RepeatForever::create(sequence);
    victoryLabel->runAction(repeatAction);
    
    // 胜利信息
    auto infoLabel = Label::createWithTTF("You successfully defended your house!", "fonts/Marker Felt.ttf", 32);
    infoLabel->setPosition(centerX, centerY + 20);
    infoLabel->setColor(Color3B::WHITE);
    this->addChild(infoLabel, 1);
    
    // 下一关按钮（暂时重新开始）
    auto nextLevelButton = ui::Button::create();
    nextLevelButton->setTitleText("Next Level");
    nextLevelButton->setTitleFontName("fonts/Marker Felt.ttf");
    nextLevelButton->setTitleFontSize(32);
    nextLevelButton->setTitleColor(Color3B::WHITE);
    nextLevelButton->setColor(Color3B(0, 150, 0));
    nextLevelButton->setPosition(Vec2(centerX - 100, centerY - 80));
    nextLevelButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onNextLevelButtonClicked(sender);
        }
    });
    this->addChild(nextLevelButton, 1);
    
    // 返回主菜单按钮
    auto mainMenuButton = ui::Button::create();
    mainMenuButton->setTitleText("Main Menu");
    mainMenuButton->setTitleFontName("fonts/Marker Felt.ttf");
    mainMenuButton->setTitleFontSize(32);
    mainMenuButton->setTitleColor(Color3B::WHITE);
    mainMenuButton->setColor(Color3B(100, 100, 0));
    mainMenuButton->setPosition(Vec2(centerX + 100, centerY - 80));
    mainMenuButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onMainMenuButtonClicked(sender);
        }
    });
    this->addChild(mainMenuButton, 1);
}

void VictoryScene::onNextLevelButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Next level button clicked");
    // TODO: 实现关卡选择或直接开始下一关
    SceneManager::getInstance().restartGame();
}

void VictoryScene::onMainMenuButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Main menu button clicked");
    SceneManager::getInstance().gotoStartScene();
}