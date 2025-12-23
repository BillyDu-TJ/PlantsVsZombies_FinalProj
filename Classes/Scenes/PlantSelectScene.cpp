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
                card->setScale(1.2f); // Initial card scale
                
                // Cards don't need individual event listeners, handled by global mouse listener
                
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
        
        // Check if clicked on any card (reverse order to check top cards first)
        for (auto it = _availableCards.rbegin(); it != _availableCards.rend(); ++it) {
            auto* card = *it;
            
            // Get card's world bounding box
            Vec2 cardWorldPos = _cardArea->convertToWorldSpace(card->getPosition());
            Size cardSize = card->getContentSize();
            float scale = card->getScale();
            
            Rect cardRect(
                cardWorldPos.x - cardSize.width * scale * 0.5f,
                cardWorldPos.y - cardSize.height * scale * 0.5f,
                cardSize.width * scale,
                cardSize.height * scale
            );
            
            if (cardRect.containsPoint(worldPos)) {
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
    
    // Visual feedback: make card bigger
    card->setScale(1.5f);
    
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
        
        // Restore original card size
        card->setScale(1.2f);
        
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

