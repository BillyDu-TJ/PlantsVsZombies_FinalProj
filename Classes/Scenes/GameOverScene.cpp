// ��Ϸʧ�ܳ���ʵ��
// 2025.12.15 by BillyDu
#include "GameOverScene.h"
#include "../Managers/SceneManager.h"
#include "../Managers/AudioManager.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

Scene* GameOverScene::createScene() {
    return GameOverScene::create();
}

bool GameOverScene::init() {
    if (!Scene::init()) {
        return false;
    }
    
    _visibleSize = Director::getInstance()->getVisibleSize();
    _origin = Director::getInstance()->getVisibleOrigin();
    
    createBackground();
    createUI();
    
    return true;
}

void GameOverScene::createBackground() {
    // ����ɫ������ʾʧ��
    auto bg = LayerColor::create(Color4B(80, 0, 0, 200));
    this->addChild(bg, -1);
}

void GameOverScene::createUI() {
    float centerX = _visibleSize.width/2 + _origin.x;
    float centerY = _visibleSize.height/2 + _origin.y;
    
    // "Game Over" ����
    auto gameOverLabel = Label::createWithTTF("GAME OVER", "fonts/Marker Felt.ttf", 72);
    gameOverLabel->setPosition(centerX, centerY + 100);
    gameOverLabel->setColor(Color3B::RED);
    this->addChild(gameOverLabel, 1);
    
    // ʧ����Ϣ
    auto infoLabel = Label::createWithTTF("The zombies ate your brains!", "fonts/Marker Felt.ttf", 32);
    infoLabel->setPosition(centerX, centerY + 20);
    infoLabel->setColor(Color3B::WHITE);
    this->addChild(infoLabel, 1);
    
    // ���¿�ʼ��ť
    auto restartButton = ui::Button::create();
    restartButton->setTitleText("Try Again");
    restartButton->setTitleFontName("fonts/Marker Felt.ttf");
    restartButton->setTitleFontSize(32);
    restartButton->setTitleColor(Color3B::WHITE);
    restartButton->setColor(Color3B(0, 100, 0));
    restartButton->setPosition(Vec2(centerX - 100, centerY - 80));
    restartButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onRestartButtonClicked(sender);
        }
    });
    this->addChild(restartButton, 1);
    
    // �������˵���ť
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

void GameOverScene::onRestartButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Restart button clicked");
    SceneManager::getInstance().restartGame();
}

void GameOverScene::onMainMenuButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Main menu button clicked");
    SceneManager::getInstance().gotoStartScene();
}