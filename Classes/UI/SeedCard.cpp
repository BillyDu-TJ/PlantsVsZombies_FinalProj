// 种子卡片类 - 使用PNG资源
// 2025.12.15 by BillyDu
// 更新：实现冷却倒计时显示功能，种植后显示倒计时数字，冷却期间禁用种植
// by Zhao.12.23
#include "SeedCard.h"
#include "../Managers/DataManager.h"
#include <cmath>

USING_NS_CC;

SeedCard* SeedCard::create(int plantId) {
    SeedCard* ret = new (std::nothrow) SeedCard();
    if (ret && ret->init(plantId)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SeedCard::init(int plantId) {
    if (!Node::init()) return false;
    _plantId = plantId;

    // 1. 获取植物数据
    try {
        auto data = DataManager::getInstance().getPlantData(plantId);
        _cost = data.cost;
        
        // 2. 确定卡片背景路径 - 优先使用配置中的cardImage
        // 如果配置中没有cardImage，则使用默认命名规则：card_1001.png, card_1002.png 等
        // 现在优先使用配置中的cardImage，如果没有则使用默认命名规则
        std::string cardBgPath;
        if (!data.cardImage.empty()) {
            cardBgPath = data.cardImage;
        } else {
            // 默认命名规则：card_1001.png, card_1002.png 等
            cardBgPath = "cards/card_" + std::to_string(plantId) + ".png";
        }
        
        // 3. 尝试加载卡片背景图片
        if (FileUtils::getInstance()->isFileExist(cardBgPath)) {
            _bg = Sprite::create(cardBgPath);
            if (_bg) {
                // 调整背景图片大小到 72x96 (保持原始UI尺寸)
                Size originalSize = _bg->getContentSize();
                float scaleX = 72.0f / originalSize.width;
                float scaleY = 96.0f / originalSize.height;
                
                // 使用较小的缩放比例，确保图片完整显示且不变形（取scaleX和scaleY的较小值）
                float scale = std::min(scaleX, scaleY);
                _bg->setScale(scale);
                
                // 设置背景锚点和位置
                _bg->setAnchorPoint(Vec2(0, 0));
                _bg->setPosition(0, 0);
                this->addChild(_bg, 0);
                
                CCLOG("[Info] SeedCard loaded PNG asset: %s (Scale: %.2f)", cardBgPath.c_str(), scale);
            }
        }
        else {
            // 如果卡片背景不存在，尝试使用通用卡片背景 + 植物图标
            std::string genericCardPath = "cards/card_generic.png";
            
            if (FileUtils::getInstance()->isFileExist(genericCardPath)) {
                _bg = Sprite::create(genericCardPath);
                if (_bg) {
                    Size originalSize = _bg->getContentSize();
                    float scaleX = 72.0f / originalSize.width;
                    float scaleY = 96.0f / originalSize.height;
                    float scale = std::min(scaleX, scaleY);
                    
                    _bg->setScale(scale);
                    _bg->setAnchorPoint(Vec2(0, 0));
                    _bg->setPosition(0, 0);
                    this->addChild(_bg, 0);
                    
                    // 在通用卡片上添加植物图标
                    if (FileUtils::getInstance()->isFileExist(data.texturePath)) {
                        _icon = Sprite::create(data.texturePath);
                        if (_icon) {
                            // 调整植物图标大小以适应卡片
                            float iconScale = 45.0f / std::max(_icon->getContentSize().width, _icon->getContentSize().height);
                            _icon->setScale(iconScale);
                            _icon->setPosition(36, 60); // 居中位置
                            this->addChild(_icon, 1);
                        }
                    }
                    
                    CCLOG("[Info] SeedCard using generic card with plant icon");
                }
            }
            else {
                // 如果通用卡片也不存在，则绘制一个简单的灰色背景
                auto bgLayer = LayerColor::create(Color4B(100, 100, 100, 200), 72, 96);
                this->addChild(bgLayer, 0);
                
                if (FileUtils::getInstance()->isFileExist(data.texturePath)) {
                    _icon = Sprite::create(data.texturePath);
                    if (_icon) {
                        float scale = 50.0f / std::max(_icon->getContentSize().width, _icon->getContentSize().height);
                        _icon->setScale(scale);
                        _icon->setPosition(36, 50);
                        this->addChild(_icon, 1);
                    }
                }
                
                CCLOG("[Info] SeedCard using fallback drawn background");
            }
        }
        
        // 4. 设置卡片内容区域大小
        this->setContentSize(Size(72, 96));
        
        // 5. 创建价格标签 (如果背景不存在或卡片背景路径不存在，则显示价格标签)
        if (!_bg || !FileUtils::getInstance()->isFileExist(cardBgPath)) {
            _costLabel = Label::createWithSystemFont(std::to_string(_cost), "Arial", 14);
            _costLabel->setPosition(36, 10); // 底部居中
            _costLabel->setColor(Color3B::WHITE);
            _costLabel->enableOutline(Color4B::BLACK, 1); // 添加黑色描边以提高可读性
            this->addChild(_costLabel, 2);
        }
        
        // 6. 创建冷却倒计时标签（初始隐藏）
        _cooldownLabel = Label::createWithSystemFont("", "Arial", 32);
        _cooldownLabel->setPosition(36, 48); // 卡片中心
        _cooldownLabel->setColor(Color3B::BLACK);
        _cooldownLabel->enableOutline(Color4B::WHITE, 2); // 添加白色描边以提高可读性
        _cooldownLabel->setVisible(false);
        this->addChild(_cooldownLabel, 10); // 最高层级，确保显示在最上层
        
        // 7. 添加触摸事件监听
        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [this](Touch* touch, Event* event) {
            Vec2 p = this->convertToNodeSpace(touch->getLocation());
            Rect rect = Rect(0, 0, this->getContentSize().width, this->getContentSize().height);

            if (rect.containsPoint(p)) {
                if (_onSelectCallback) {
                    _onSelectCallback(_plantId);
                }
                // 添加点击动画效果
                this->runAction(Sequence::create(
                    ScaleTo::create(0.1f, 0.95f), 
                    ScaleTo::create(0.1f, 1.0f), 
                    nullptr));
                return true;
            }
            return false;
        };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
        
    }
    catch (const std::exception& e) {
        CCLOG("[Err] SeedCard init failed for plant %d: %s", plantId, e.what());
        return false;
    }

    return true;
}

void SeedCard::updateSunCheck(int currentSun) {
    this->setCascadeOpacityEnabled(true);
    
    // 根据当前阳光值更新卡片显示状态
    if (currentSun < _cost) {
        this->setOpacity(100);  // 阳光不足时变暗
        
        // 同时调整背景和图标颜色为灰色
        if (_bg) {
            _bg->setColor(Color3B(128, 128, 128)); // 灰色
        }
        if (_icon) {
            _icon->setColor(Color3B(128, 128, 128)); // 图标也变灰
        }
    }
    else {
        this->setOpacity(255);  // 阳光充足时恢复正常
        
        // 恢复正常颜色
        if (_bg) {
            _bg->setColor(Color3B::WHITE);
        }
        if (_icon) {
            _icon->setColor(Color3B::WHITE);
        }
    }
}

void SeedCard::setOnSelectCallback(const std::function<void(int)>& callback) {
    _onSelectCallback = callback;
}

void SeedCard::startCooldown(float cooldownTime) {
    _cooldownTotal = cooldownTime;
    _cooldownRemaining = cooldownTime;
    _cooldownLabel->setVisible(true);
    updateCooldown(0.0f); // 立即更新一次显示
}

void SeedCard::updateCooldown(float dt) {
    if (_cooldownRemaining > 0.0f) {
        _cooldownRemaining -= dt;
        
        if (_cooldownRemaining <= 0.0f) {
            _cooldownRemaining = 0.0f;
            _cooldownLabel->setVisible(false);
        } else {
            // 显示倒计时（向上取整）
            int seconds = static_cast<int>(std::ceil(_cooldownRemaining));
            _cooldownLabel->setString(std::to_string(seconds));
            _cooldownLabel->setVisible(true);
        }
    }
}
