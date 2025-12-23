// ��Ϸʤ������ʵ��
// 2025.12.15 by BillyDu
#include "VictoryScene.h"
#include "../Managers/SceneManager.h"
#include "../Managers/LevelManager.h"
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
    // ���ɫ������ʾʤ��
    auto bg = LayerGradient::create(Color4B(255, 215, 0, 255), Color4B(255, 140, 0, 255));
    this->addChild(bg, -1);
}

void VictoryScene::createUI() {
    float centerX = _visibleSize.width/2 + _origin.x;
    float centerY = _visibleSize.height/2 + _origin.y;
    
    // "Victory!" ����
    auto victoryLabel = Label::createWithTTF("VICTORY!", "fonts/Marker Felt.ttf", 72);
    victoryLabel->setPosition(centerX, centerY + 100);
    victoryLabel->setColor(Color3B::YELLOW);
    this->addChild(victoryLabel, 1);
    
    // ����ʤ������
    auto scaleUp = ScaleTo::create(0.5f, 1.2f);
    auto scaleDown = ScaleTo::create(0.5f, 1.0f);
    auto sequence = Sequence::create(scaleUp, scaleDown, nullptr);
    auto repeatAction = RepeatForever::create(sequence);
    victoryLabel->runAction(repeatAction);
    
    // 获取当前地图ID，判断是否是最后一关
    int currentMapId = SceneManager::getInstance().getCurrentMapId();
    bool isFinalLevel = (currentMapId == 4);
    
    // ʤ����Ϣ（根据是否是最后一关显示不同内容）
    std::string infoText = isFinalLevel ? 
        "Congratulations! You completed all levels!" : 
        "You successfully defended your house!";
    auto infoLabel = Label::createWithTTF(infoText, "fonts/Marker Felt.ttf", 32);
    infoLabel->setPosition(centerX, centerY + 20);
    infoLabel->setColor(Color3B::WHITE);
    this->addChild(infoLabel, 1);
    
    // 如果不是最后一关，显示"下一关"按钮
    if (!isFinalLevel) {
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
    }
    
    // �������˵���ť（最后一关显示"Exit"，其他显示"Main Menu"）
    auto mainMenuButton = ui::Button::create();
    mainMenuButton->setTitleText(isFinalLevel ? "Exit" : "Main Menu");
    mainMenuButton->setTitleFontName("fonts/Marker Felt.ttf");
    mainMenuButton->setTitleFontSize(32);
    mainMenuButton->setTitleColor(Color3B::WHITE);
    mainMenuButton->setColor(Color3B(100, 100, 0));
    mainMenuButton->setPosition(Vec2(isFinalLevel ? centerX : centerX + 100, centerY - 80));
    mainMenuButton->addTouchEventListener([this, isFinalLevel](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            if (isFinalLevel) {
                // 最后一关：退出游戏
                Director::getInstance()->end();
            } else {
                this->onMainMenuButtonClicked(sender);
            }
        }
    });
    this->addChild(mainMenuButton, 1);
}

void VictoryScene::onNextLevelButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Next level button clicked");
    
    // 获取当前地图ID，跳转到下一关
    int currentMapId = SceneManager::getInstance().getCurrentMapId();
    int nextMapId = currentMapId + 1;
    
    if (nextMapId > 4) {
        // 不应该到这里（最后一关不会显示Next Level按钮）
        CCLOG("[Warn] Already at final level, returning to main menu");
        SceneManager::getInstance().gotoStartScene();
        return;
    }
    
    CCLOG("[Info] Proceeding to Map %d", nextMapId);
    
    // 设置下一关的地图ID
    SceneManager::getInstance().setCurrentMapId(nextMapId);
    
    // 设置下一关的背景路径
    std::string bgPath;
    switch (nextMapId) {
        case 1: bgPath = "bg/day.jpg"; break;
        case 2: bgPath = "bg/day2.jpg"; break;
        case 3: bgPath = "bg/night.jpg"; break;
        case 4: bgPath = "bg/night2.jpg"; break;
        default: bgPath = "bg/day.jpg"; break;
    }
    
    // 设置LevelManager的背景路径
    LevelManager::getInstance().setBackgroundPath(bgPath);
    
    // 加载对应地图的关卡配置
    std::string levelFile;
    if (nextMapId == 2) {
        levelFile = "data/level_map2.json";
    } else if (nextMapId == 4) {
        levelFile = "data/level_map4.json";
    } else {
        levelFile = "data/level_test.json";
    }
    
    try {
        LevelManager::getInstance().loadLevel(levelFile);
    } catch (...) {
        CCLOG("[Warn] Failed to load level config for map %d", nextMapId);
    }
    
    // 直接进入植物选择场景（跳过地图选择）
    SceneManager::getInstance().gotoPlantSelectScene();
}

void VictoryScene::onMainMenuButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Main menu button clicked");
    SceneManager::getInstance().gotoStartScene();
}