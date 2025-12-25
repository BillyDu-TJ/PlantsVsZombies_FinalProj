// 植物选择场景实现
// 2025.12.21 by Zhao
#include "PlantSelectScene.h"
#include "../Managers/SceneManager.h"
#include "../Managers/DataManager.h"
#include "../Managers/AudioManager.h"
#include "../UI/SeedCard.h"
#include "../Consts.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

Scene* PlantSelectScene::createScene() {
    return PlantSelectScene::create();
}

bool PlantSelectScene::init() {
    if (!Scene::init()) {
        return false;
    }
    
    _visibleSize = Director::getInstance()->getVisibleSize();
    _origin = Director::getInstance()->getVisibleOrigin();
    
    _selectedPlantIds.clear();
    //_barCards.clear();
    _availableCards.clear();
    
    // Load data first before creating cards
    try {
        DataManager::getInstance().loadData();
        CCLOG("[Info] Data loaded successfully for plant selection");
    } catch (const std::exception& e) {
        CCLOG("[Err] Failed to load data: %s", e.what());
        return false;
    }
    
    createBackground();
    createTitle();
    createCardArea();
    createConfirmButton();
    createGlobalMouseListener();
    
    return true;
}

void PlantSelectScene::createBackground() {
    // Use simple dark background
    auto bg = LayerColor::create(Color4B(50, 50, 50, 255), _visibleSize.width, _visibleSize.height);
    bg->setPosition(_origin);
    this->addChild(bg, -1);
}

void PlantSelectScene::createTitle() {
    // Create title "Select Your Plants"
    auto title = Label::createWithTTF("Select Your Plants", "fonts/Marker Felt.ttf", 48);
    title->setPosition(_visibleSize.width / 2 + _origin.x, _visibleSize.height * 0.85f + _origin.y);
    title->setColor(Color3B::YELLOW);
    this->addChild(title, 10);
    
    // Create selected count label
    _selectedCountLabel = Label::createWithTTF("Selected: 0/8", "fonts/Marker Felt.ttf", 24);
    _selectedCountLabel->setPosition(_visibleSize.width / 2 + _origin.x, _visibleSize.height * 0.75f + _origin.y);
    _selectedCountLabel->setColor(Color3B::WHITE);
    this->addChild(_selectedCountLabel, 10);
}

void PlantSelectScene::createCardArea() {
    // Create card area container
    _cardArea = Node::create();
    _cardArea->setPosition(_origin);
    this->addChild(_cardArea, 5); // Higher z-order to be visible on PanelBackground
    
    // Get all available plant IDs（基础池）
    std::vector<int> basePlantIds = {
        1001, 1002, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014
    };

    // 根据当前地图（章节）调整可选植物：夜晚地图禁止使用向日葵（1002）
    int currentMapId = SceneManager::getInstance().getCurrentMapId();
    bool isNightMap = (currentMapId == 3 || currentMapId == 4);

    std::vector<int> availablePlantIds;
    availablePlantIds.reserve(basePlantIds.size());
    for (int id : basePlantIds) {
        // 夜晚地图禁止向日葵，鼓励使用阳光菇
        if (isNightMap && id == 1002) {
            continue;
        }
        availablePlantIds.push_back(id);
    }
    
    // Calculate card positions - arrange them in a single row, centered
    float cardGap = 90.0f; // Card spacing
    float centerX = _visibleSize.width / 2 + _origin.x;
    float centerY = _visibleSize.height * 0.5f + _origin.y;
    
    // Calculate starting X position to center all cards
    float totalWidth = (availablePlantIds.size() - 1) * cardGap;
    float startX = centerX - totalWidth / 2.0f;
    
    // Create all available cards
    for (size_t i = 0; i < availablePlantIds.size(); ++i) {
        int plantId = availablePlantIds[i];
        
        try {
            auto card = SeedCard::create(plantId);
            if (card) {
                // Calculate card position in a single row
                float x = startX + i * cardGap;
                float y = centerY;
                
                card->setPosition(x, y);
                card->setScale(1.2f); // 统一初始缩放
                
                // 重要：清除SeedCard自带的触摸监听器，避免冲突
                card->getEventDispatcher()->removeEventListenersForTarget(card);
                
                _cardArea->addChild(card);
                _availableCards.push_back(card);
                _cardSelectedMap[card] = false; // Initialize as not selected
                
                CCLOG("[Info] Created card for plant %d at (%.1f, %.1f)", plantId, x, y);
            } else {
                CCLOG("[Err] Failed to create card for plant %d: SeedCard::create returned nullptr", plantId);
            }
        } catch (const std::exception& e) {
            CCLOG("[Err] Failed to create card for plant %d: %s", plantId, e.what());
        }
    }
    
    CCLOG("[Info] Created %zu cards in card area", _availableCards.size());
}

void PlantSelectScene::createGlobalMouseListener() {
    // Create global mouse listener for card selection with better hit detection
    _globalMouseListener = EventListenerMouse::create();
    _globalMouseListener->onMouseDown = [this](EventMouse* event) {
        Vec2 location = event->getLocationInView();
        Vec2 worldPos = Director::getInstance()->convertToGL(location);
        
        // Debug: 打印鼠标点击位置
        CCLOG("[Debug] Mouse clicked at: (%.1f, %.1f)", worldPos.x, worldPos.y);
        
        // Check if clicked on any card (reverse order to check top cards first)
        for (auto it = _availableCards.rbegin(); it != _availableCards.rend(); ++it) {
            auto* card = *it;
            
            // 获取卡片在世界坐标系中的位置
            Vec2 cardLocalPos = card->getPosition();
            Vec2 cardWorldPos = _cardArea->convertToWorldSpace(cardLocalPos);
            Size cardSize = card->getContentSize();
            float scale = card->getScale();
            
            // 关键修复：考虑到SeedCard内部使用的是Vec2(0,0)锚点（左下角）
            // 但是Node默认锚点是Vec2(0.5, 0.5)（中心），所以需要调整计算
            // SeedCard的setContentSize设置为Size(72, 96)
            float actualWidth = cardSize.width * scale;
            float actualHeight = cardSize.height * scale;
            
            // 计算实际的碰撞矩形（以卡片中心为基准）
            Rect cardRect(
                cardWorldPos.x - actualWidth * 0.5f,    // 左边界
                cardWorldPos.y - actualHeight * 0.5f,   // 下边界
                actualWidth,                             // 宽度
                actualHeight                             // 高度
            );
            
            // Debug: 打印卡片信息
            CCLOG("[Debug] Card %d: LocalPos(%.1f, %.1f), WorldPos(%.1f, %.1f), Size(%.1fx%.1f), Scale(%.2f), Rect(%.1f,%.1f,%.1f,%.1f)", 
                  dynamic_cast<SeedCard*>(card)->getPlantId(),
                  cardLocalPos.x, cardLocalPos.y,
                  cardWorldPos.x, cardWorldPos.y,
                  cardSize.width, cardSize.height, scale,
                  cardRect.origin.x, cardRect.origin.y, cardRect.size.width, cardRect.size.height);
            
            if (cardRect.containsPoint(worldPos)) {
                CCLOG("[Debug] Hit detected on card %d", dynamic_cast<SeedCard*>(card)->getPlantId());
                
                if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
                    // Left click: select card
                    this->onCardLeftClick(event, card);
                } else if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
                    // Right click: deselect card
                    this->onCardRightClick(event, card);
                }
                return;
            }
        }
        
        CCLOG("[Debug] No card hit");
    };
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_globalMouseListener, this);
}

void PlantSelectScene::createConfirmButton() {
    // Create Start button
    auto confirmButton = ui::Button::create();
    confirmButton->setTitleText("Start");
    confirmButton->setTitleFontName("fonts/Marker Felt.ttf");
    confirmButton->setTitleFontSize(32);
    confirmButton->setTitleColor(Color3B::WHITE);
    confirmButton->setColor(Color3B(0, 150, 0));
    confirmButton->setContentSize(Size(150, 60));
    
    float buttonX = _visibleSize.width * 0.85f + _origin.x;
    float buttonY = _visibleSize.height * 0.15f + _origin.y;
    confirmButton->setPosition(Vec2(buttonX, buttonY));
    
    confirmButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onConfirmButtonClicked(sender);
        }
    });
    
    this->addChild(confirmButton, 10);
    
    // Create Exit button
    auto backButton = ui::Button::create();
    backButton->setTitleText("Exit");
    backButton->setTitleFontName("fonts/Marker Felt.ttf");
    backButton->setTitleFontSize(32);
    backButton->setTitleColor(Color3B::WHITE);
    backButton->setColor(Color3B(150, 0, 0));
    backButton->setContentSize(Size(150, 60));
    
    float backX = _visibleSize.width * 0.15f + _origin.x;
    backButton->setPosition(Vec2(backX, buttonY));
    
    backButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onBackButtonClicked(sender);
        }
    });
    
    this->addChild(backButton, 10);
}

void PlantSelectScene::onCardLeftClick(cocos2d::EventMouse* event, cocos2d::Node* card) {
    // Check if card is already selected
    if (_cardSelectedMap[card]) {
        CCLOG("[Info] Card already selected");
        return;
    }
    
    int plantId = dynamic_cast<SeedCard*>(card)->getPlantId();
    
    // Check if plant already in selected list
    bool alreadySelected = false;
    for (int id : _selectedPlantIds) {
        if (id == plantId) {
            alreadySelected = true;
            break;
        }
    }
    
    if (alreadySelected) {
        CCLOG("[Info] Plant %d already selected", plantId);
        return;
    }
    
    // Check if selection is full
    if (_selectedPlantIds.size() >= MAX_SELECTED_CARDS) {
        CCLOG("[Info] Selection is full, cannot add more cards");
        return;
    }
    
    // Add to selected list
    _selectedPlantIds.push_back(plantId);
    _cardSelectedMap[card] = true;
    
    // Visual feedback: 保持统一大小，添加高亮效果
    card->setColor(Color3B(255, 255, 150)); // 淡黄色高亮
    
    // 添加发光边框效果 - 修复位置和锚点
    auto glowSprite = Sprite::create();
    glowSprite->setTextureRect(Rect(0, 0, 80, 104)); // 稍大于卡片尺寸 (72x96 + 边距)
    glowSprite->setColor(Color3B::YELLOW);
    glowSprite->setOpacity(80);
    
    // 关键修复：设置锚点为左下角，与SeedCard的背景图片对齐
    glowSprite->setAnchorPoint(Vec2(0, 0));
    // 设置位置为相对于卡片左下角的偏移
    glowSprite->setPosition(-4, -4); // 向左下偏移4像素，创建边框效果
    glowSprite->setTag(999); // 用于后续移除
    card->addChild(glowSprite, -1); // 放在卡片后面作为边框
    
    // 添加轻微的点击反馈动画，但保持最终大小一致
    card->runAction(Sequence::create(
        ScaleTo::create(0.05f, 1.15f), 
        ScaleTo::create(0.05f, 1.2f), 
        nullptr));
    
    // 播放音效
    AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);
    
    // Update selected count label
    if (_selectedCountLabel) {
        char text[64];
        snprintf(text, sizeof(text), "Selected: %zu/%d", _selectedPlantIds.size(), MAX_SELECTED_CARDS);
        _selectedCountLabel->setString(text);
    }
    
    CCLOG("[Info] Card %d selected (Total: %zu)", plantId, _selectedPlantIds.size());
}

void PlantSelectScene::onCardRightClick(cocos2d::EventMouse* event, cocos2d::Node* card) {
    // Check if card is selected
    if (!_cardSelectedMap[card]) {
        CCLOG("[Info] Card not selected, cannot deselect");
        return;
    }
    
    int plantId = dynamic_cast<SeedCard*>(card)->getPlantId();
    
    // Find and remove from selected list
    auto it = std::find(_selectedPlantIds.begin(), _selectedPlantIds.end(), plantId);
    if (it != _selectedPlantIds.end()) {
        _selectedPlantIds.erase(it);
        _cardSelectedMap[card] = false;
        
        // 恢复原始状态：保持统一大小，移除高亮效果
        card->setScale(1.2f); // 确保缩放一致
        card->setColor(Color3B::WHITE); // 恢复原始颜色
        
        // 移除发光边框
        auto glowSprite = card->getChildByTag(999);
        if (glowSprite) {
            glowSprite->removeFromParent();
        }
        
        // 播放音效
        AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);
        
        // Update selected count label
        if (_selectedCountLabel) {
            char text[64];
            snprintf(text, sizeof(text), "Selected: %zu/%d", _selectedPlantIds.size(), MAX_SELECTED_CARDS);
            _selectedCountLabel->setString(text);
        }
        
        CCLOG("[Info] Card %d deselected (Total: %zu)", plantId, _selectedPlantIds.size());
    }
}

void PlantSelectScene::onConfirmButtonClicked(cocos2d::Ref* sender) {
    if (_selectedPlantIds.empty()) {
        CCLOG("[Warn] No plants selected");
        // 可以显示提示信息
        return;
    }
    
    CCLOG("[Info] Confirmed selection with %zu plants", _selectedPlantIds.size());
    
    // 将选中的植物ID列表传递给SceneManager
    SceneManager::getInstance().setSelectedPlants(_selectedPlantIds);
    
    // 播放音效
    AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);
    
    // 进入游戏场景
    SceneManager::getInstance().gotoGameScene();
}

void PlantSelectScene::onBackButtonClicked(cocos2d::Ref* sender) {
    CCLOG("[Info] Back button clicked");
    AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);
    SceneManager::getInstance().gotoMapSelectScene();
}

