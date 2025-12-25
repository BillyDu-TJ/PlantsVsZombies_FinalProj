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

    // 使用键盘数字键 1~4 选择对应地图，取消鼠标选图，避免误触
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        int mapId = -1;
        switch (keyCode) {
        case EventKeyboard::KeyCode::KEY_1:
            mapId = 1;
            break;
        case EventKeyboard::KeyCode::KEY_2:
            mapId = 2;
            break;
        case EventKeyboard::KeyCode::KEY_3:
            mapId = 3;
            break;
        case EventKeyboard::KeyCode::KEY_4:
            mapId = 4;
            break;
        default:
            break;
        }

        if (mapId != -1 && isMapUnlocked(mapId)) {
            CCLOG("[Info] Map %d selected by keyboard", mapId);
            this->onMapButtonClicked(nullptr, mapId);
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

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
    // 以屏幕中心为基准，做一个 2x2 的网格布局，让每个地图按钮之间间距更大
    float centerX = _visibleSize.width / 2 + _origin.x;
    float centerY = _visibleSize.height * 0.55f + _origin.y;

    float buttonWidth = 220.0f;
    float buttonHeight = 150.0f;

    // 水平和垂直间距：保证按钮之间有明显空隙
    float horizontalSpacing = 260.0f;
    float verticalSpacing = 220.0f;
    
    // 创建4个地图按钮，排列成 2x2：
    // 视觉上我们希望：
    // [3] [4]
    // [1] [2]
    // 因此这里对 mapId 做一次手动映射，保证显示位置与点击判定一致
    for (int mapId = 1; mapId <= 4; ++mapId) {
        int row = 0;
        int col = 0;

        switch (mapId) {
        case 3: // Map1 放在左下
            row = 1; col = 0;
            break;
        case 4: // Map2 放在右下
            row = 1; col = 1;
            break;
        case 1: // Map3 放在左上
            row = 0; col = 0;
            break;
        case 2: // Map4 放在右上
            row = 0; col = 1;
            break;
        default:
            row = 0; col = 0;
            break;
        }

        // 让左列在 centerX 左侧一点，右列在右侧一点；
        // 上排在 centerY 上方，下排在下方。
        float x = centerX + (col == 0 ? -horizontalSpacing / 2 : horizontalSpacing / 2);
        float y = centerY + (row == 0 ? verticalSpacing / 2 : -verticalSpacing / 2);
        
        // 创建按钮容器
        auto buttonContainer = Node::create();
        buttonContainer->setPosition(x, y);
        buttonContainer->setContentSize(Size(buttonWidth, buttonHeight));
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
        
        // 为已解锁的地图添加鼠标点击事件
        if (isMapUnlocked(mapId)) {
            // 添加触摸事件监听器
            auto touchListener = EventListenerTouchOneByOne::create();
            touchListener->setSwallowTouches(true);
            touchListener->onTouchBegan = [this, mapId, buttonContainer, buttonBg, buttonWidth, buttonHeight](Touch* touch, Event* event) {
                Vec2 touchPos = buttonContainer->getParent()->convertToNodeSpace(touch->getLocation());
                Vec2 buttonPos = buttonContainer->getPosition();
                Rect buttonRect = Rect(buttonPos.x - buttonWidth/2, buttonPos.y - buttonHeight/2, 
                                     buttonWidth, buttonHeight);
                
                if (buttonRect.containsPoint(touchPos)) {
                    // 点击时的视觉反馈：按钮变亮
                    buttonBg->setColor(Color3B(150, 200, 150));
                    return true;
                }
                return false;
            };
            
            touchListener->onTouchEnded = [this, mapId, buttonContainer, buttonBg, buttonWidth, buttonHeight](Touch* touch, Event* event) {
                Vec2 touchPos = buttonContainer->getParent()->convertToNodeSpace(touch->getLocation());
                Vec2 buttonPos = buttonContainer->getPosition();
                Rect buttonRect = Rect(buttonPos.x - buttonWidth/2, buttonPos.y - buttonHeight/2, 
                                     buttonWidth, buttonHeight);
                
                // 恢复按钮原始颜色
                buttonBg->setColor(Color3B(100, 150, 100));
                
                if (buttonRect.containsPoint(touchPos)) {
                    // 在按钮范围内释放，触发地图选择
                    CCLOG("[Info] Map %d clicked with mouse", mapId);
                    this->onMapButtonClicked(nullptr, mapId);
                }
            };
            
            touchListener->onTouchCancelled = [buttonBg](Touch* touch, Event* event) {
                // 取消时恢复按钮颜色
                buttonBg->setColor(Color3B(100, 150, 100));
            };
            
            _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, buttonContainer);
            
            // 添加悬停效果（可选）
            auto mouseListener = EventListenerMouse::create();
            mouseListener->onMouseMove = [this, buttonContainer, buttonBg, mapId, buttonWidth, buttonHeight](EventMouse* event) {
                Vec2 mousePos = buttonContainer->getParent()->convertToNodeSpace(Vec2(event->getCursorX(), event->getCursorY()));
                Vec2 buttonPos = buttonContainer->getPosition();
                Rect buttonRect = Rect(buttonPos.x - buttonWidth/2, buttonPos.y - buttonHeight/2, 
                                     buttonWidth, buttonHeight);
                
                if (buttonRect.containsPoint(mousePos)) {
                    // 鼠标悬停时稍微变亮
                    buttonBg->setColor(Color3B(120, 170, 120));
                } else {
                    // 鼠标离开时恢复原色
                    buttonBg->setColor(Color3B(100, 150, 100));
                }
            };
            _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, buttonContainer);
            
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
    
    // 记录当前地图ID（用于难度和夜晚限制）
    SceneManager::getInstance().setCurrentMapId(mapId);

    // 设置选中的地图背景
    std::string bgPath = getMapBackgroundPath(mapId);
    
    // 先设置LevelManager中的背景路径（在loadLevel之前）
    LevelManager::getInstance().setBackgroundPath(bgPath);
    CCLOG("[Info] Map %d selected, background set to: %s", mapId, bgPath.c_str());
    
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

