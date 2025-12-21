// 地图选择场景实现
// 2025.12.2 by BillyDu
#include "MapSelectScene.h"
#include "../Managers/SceneManager.h"
#include "../Managers/LevelManager.h"
#include "../Managers/AudioManager.h"
#include "ui/CocosGUI.h"
#include "json/document.h"
#include "../Utils/GameException.h"

USING_NS_CC;
using namespace rapidjson;

Scene* MapSelectScene::createScene() {
    return MapSelectScene::create();
}

bool MapSelectScene::init() {
    if (!Scene::init()) {
        return false;
    }
    
    _visibleSize = Director::getInstance()->getVisibleSize();
    _origin = Director::getInstance()->getVisibleOrigin();
    
    createBackground();
    createTitle();
    createMapButtons();
    createBackButton();
    
    return true;
}

void MapSelectScene::createBackground() {
    auto bg = Sprite::create("bg/menu.png");
    if (bg) {
        bg->setPosition(_visibleSize.width/2 + _origin.x, _visibleSize.height/2 + _origin.y);
        this->addChild(bg, -1);
    } else {
        // 如果没有背景图，使用纯色背景
        auto layer = LayerColor::create(Color4B(50, 100, 50, 255));
        this->addChild(layer, -1);
    }
}

void MapSelectScene::createTitle() {
    auto title = Label::createWithTTF("Select Map", "fonts/Marker Felt.ttf", 48);
    title->setPosition(_visibleSize.width/2 + _origin.x, _visibleSize.height * 0.85f + _origin.y);
    title->setColor(Color3B::YELLOW);
    this->addChild(title, 1);
}

void MapSelectScene::createMapButtons() {
    float centerX = _visibleSize.width/2 + _origin.x;
    float startY = _visibleSize.height * 0.55f + _origin.y;
    float buttonSpacing = 120.0f;
    float buttonWidth = 200.0f;
    float buttonHeight = 150.0f;
    
    // 创建4个地图按钮
    for (int mapId = 1; mapId <= 4; ++mapId) {
        float x = centerX - (4 - mapId) * buttonSpacing + buttonSpacing * 1.5f;
        float y = startY;
        
        // 创建按钮容器
        auto buttonContainer = Node::create();
        buttonContainer->setPosition(x, y);
        this->addChild(buttonContainer, 1);
        
        // 创建按钮背景
        auto buttonBg = Sprite::create();
        buttonBg->setTextureRect(Rect(0, 0, buttonWidth, buttonHeight));
        buttonBg->setColor(isMapUnlocked(mapId) ? Color3B(100, 150, 100) : Color3B(80, 80, 80));
        buttonBg->setOpacity(200);
        buttonContainer->addChild(buttonBg, 0);
        
        // 创建地图预览图（如果有的话）
        std::string previewPath = getMapBackgroundPath(mapId);
        if (isMapUnlocked(mapId) && FileUtils::getInstance()->isFileExist(previewPath)) {
            auto preview = Sprite::create(previewPath);
            if (preview) {
                // 缩放预览图以适应按钮
                float scaleX = (buttonWidth - 20) / preview->getContentSize().width;
                float scaleY = (buttonHeight - 40) / preview->getContentSize().height;
                float scale = std::min(scaleX, scaleY);
                preview->setScale(scale);
                preview->setPosition(0, 10);
                preview->setOpacity(180);
                buttonContainer->addChild(preview, 1);
            }
        }
        
        // 创建地图标签
        std::string mapText = isMapUnlocked(mapId) ? 
            StringUtils::format("Map %d", mapId) : "Locked";
        auto mapLabel = Label::createWithTTF(mapText, "fonts/Marker Felt.ttf", 24);
        mapLabel->setPosition(0, -buttonHeight/2 + 25);
        mapLabel->setColor(isMapUnlocked(mapId) ? Color3B::WHITE : Color3B::GRAY);
        buttonContainer->addChild(mapLabel, 2);
        
        // 创建可点击按钮（覆盖整个区域）
        if (isMapUnlocked(mapId)) {
            // 使用 ui::Button 支持触摸和鼠标
            auto button = ui::Button::create();
            button->setContentSize(Size(buttonWidth, buttonHeight));
            button->setTitleText("");
            button->setPosition(Vec2::ZERO);
            button->setAnchorPoint(Vec2(0.5f, 0.5f));
            button->setOpacity(0); // 透明，但可点击
            button->setSwallowTouches(true);
            button->setEnabled(true);
            button->setBright(true); // 确保按钮可交互
            
            // 添加悬停效果和点击事件
            button->addTouchEventListener([this, buttonContainer, mapId](Ref* sender, ui::Widget::TouchEventType type) {
                if (type == ui::Widget::TouchEventType::BEGAN) {
                    buttonContainer->runAction(ScaleTo::create(0.1f, 1.1f));
                    CCLOG("[Debug] Button touch began for map %d", mapId);
                } else if (type == ui::Widget::TouchEventType::ENDED) {
                    buttonContainer->runAction(ScaleTo::create(0.1f, 1.0f));
                    CCLOG("[Debug] Button touch ended for map %d", mapId);
                    this->onMapButtonClicked(sender, mapId);
                } else if (type == ui::Widget::TouchEventType::CANCELED) {
                    buttonContainer->runAction(ScaleTo::create(0.1f, 1.0f));
                }
            });
            buttonContainer->addChild(button, 10);
            
            // 添加鼠标事件监听（确保鼠标点击也能工作）
            auto mouseListener = EventListenerMouse::create();
            mouseListener->onMouseDown = [this, mapId, x, y, buttonWidth, buttonHeight, buttonContainer](EventMouse* event) {
                if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
                    Vec2 location = event->getLocationInView();
                    Vec2 worldPos = Director::getInstance()->convertToGL(location);
                    
                    // 检查点击是否在按钮范围内（使用世界坐标）
                    Rect buttonRect(x - buttonWidth/2, y - buttonHeight/2, buttonWidth, buttonHeight);
                    if (buttonRect.containsPoint(worldPos)) {
                        CCLOG("[Debug] Mouse clicked on map %d at world pos (%.1f, %.1f)", mapId, worldPos.x, worldPos.y);
                        // 添加点击反馈
                        buttonContainer->runAction(Sequence::create(
                            ScaleTo::create(0.05f, 1.1f),
                            ScaleTo::create(0.05f, 1.0f),
                            CallFunc::create([this, mapId]() {
                                this->onMapButtonClicked(nullptr, mapId);
                            }),
                            nullptr
                        ));
                    }
                }
            };
            this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mouseListener, buttonContainer);
        } else {
            // 未解锁的地图显示锁定图标
            auto lockLabel = Label::createWithTTF("🔒", "fonts/Marker Felt.ttf", 40);
            lockLabel->setPosition(0, 0);
            buttonContainer->addChild(lockLabel, 3);
        }
    }
}

void MapSelectScene::createBackButton() {
    auto backButton = ui::Button::create();
    backButton->setTitleText("Back");
    backButton->setTitleFontName("fonts/Marker Felt.ttf");
    backButton->setTitleFontSize(28);
    backButton->setTitleColor(Color3B::WHITE);
    backButton->setColor(Color3B(100, 0, 0));
    backButton->setPosition(Vec2(_visibleSize.width * 0.1f + _origin.x, 
                                  _visibleSize.height * 0.9f + _origin.y));
    backButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onBackButtonClicked(sender);
        }
    });
    this->addChild(backButton, 1);
}

void MapSelectScene::onMapButtonClicked(cocos2d::Ref* sender, int mapId) {
    CCLOG("[Info] Map %d selected", mapId);
    AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);
    
    // 设置选中的地图背景
    std::string bgPath = getMapBackgroundPath(mapId);
    
    // 先设置LevelManager中的背景路径（在loadLevel之前）
    LevelManager::getInstance().setBackgroundPath(bgPath);
    CCLOG("[Info] Map %d selected, background set to: %s", mapId, bgPath.c_str());
    
    // 加载默认的level配置（loadLevel现在不会覆盖已设置的背景路径）
    try {
        LevelManager::getInstance().loadLevel("data/level_test.json");
    } catch (...) {
        CCLOG("[Warn] Failed to load default level config");
    }
    
    // 进入植物选择场景
    SceneManager::getInstance().gotoPlantSelectScene();
}

void MapSelectScene::onBackButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Back button clicked");
    AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);
    SceneManager::getInstance().gotoStartScene();
}

std::string MapSelectScene::getMapBackgroundPath(int mapId) const {
    switch (mapId) {
        case 1:
            return "bg/day.jpg";
        case 2:
            return "bg/day2.jpg";
        case 3:
            return "bg/night.jpg";
        case 4:
            return "bg/night2.jpg";
        default:
            return "bg/day.jpg";
    }
}

bool MapSelectScene::isMapUnlocked(int mapId) const {
    // 所有地图都已解锁
    return mapId >= 1 && mapId <= 4;
}

