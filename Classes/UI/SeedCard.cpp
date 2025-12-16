// 种植卡片类实现 - 使用PNG素材
// 2025.12.15 by BillyDu
#include "SeedCard.h"
#include "../Managers/DataManager.h"

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

    // 1. 获取数据
    try {
        auto data = DataManager::getInstance().getPlantData(plantId);
        _cost = data.cost;
        
        // 2. 生成卡片背景图路径 - 基于植物ID
        // 假设卡片命名规则：card_1001.png, card_1002.png 等
        std::string cardBgPath = "cards/card_" + std::to_string(plantId) + ".png";
        
        // 3. 尝试加载卡片背景图
        if (FileUtils::getInstance()->isFileExist(cardBgPath)) {
            _bg = Sprite::create(cardBgPath);
            if (_bg) {
                // 调整卡片大小到 72x96 (保持当前UI布局)
                Size originalSize = _bg->getContentSize();
                float scaleX = 72.0f / originalSize.width;
                float scaleY = 96.0f / originalSize.height;
                
                // 使用统一缩放保持宽高比，如果需要完全填充可以用不同的scaleX和scaleY
                float scale = std::min(scaleX, scaleY);
                _bg->setScale(scale);
                
                // 设置锚点和位置
                _bg->setAnchorPoint(Vec2(0, 0));
                _bg->setPosition(0, 0);
                this->addChild(_bg, 0);
                
                CCLOG("[Info] SeedCard loaded PNG asset: %s (Scale: %.2f)", cardBgPath.c_str(), scale);
            }
        }
        else {
            // 备用方案：如果没有专门的卡片图，使用通用卡片背景 + 植物图标
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
                    
                    // 在通用背景上添加植物图标
                    if (FileUtils::getInstance()->isFileExist(data.texturePath)) {
                        _icon = Sprite::create(data.texturePath);
                        if (_icon) {
                            // 将植物图标缩放到适合卡片的大小
                            float iconScale = 45.0f / std::max(_icon->getContentSize().width, _icon->getContentSize().height);
                            _icon->setScale(iconScale);
                            _icon->setPosition(36, 60); // 卡片中心偏上
                            this->addChild(_icon, 1);
                        }
                    }
                    
                    CCLOG("[Info] SeedCard using generic card with plant icon");
                }
            }
            else {
                // 最后的备用方案：绘制简单背景（保持原有逻辑）
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
        
        // 4. 设置整个节点的内容大小
        this->setContentSize(Size(72, 96));
        
        // 5. 添加消耗文字 (仅在没有完整卡片图时显示，因为完整卡片图应该包含消耗数字)
        if (!_bg || !FileUtils::getInstance()->isFileExist(cardBgPath)) {
            _costLabel = Label::createWithSystemFont(std::to_string(_cost), "Arial", 14);
            _costLabel->setPosition(36, 10); // 底部居中
            _costLabel->setColor(Color3B::WHITE);
            _costLabel->enableOutline(Color4B::BLACK, 1); // 添加轮廓以提高可读性
            this->addChild(_costLabel, 2);
        }
        
        // 6. 添加触摸监听
        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [this](Touch* touch, Event* event) {
            Vec2 p = this->convertToNodeSpace(touch->getLocation());
            Rect rect = Rect(0, 0, this->getContentSize().width, this->getContentSize().height);

            if (rect.containsPoint(p)) {
                if (_onSelectCallback) {
                    _onSelectCallback(_plantId);
                }
                // 点击反馈动画
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
    
    // 根据阳光数量调整卡片的可用状态
    if (currentSun < _cost) {
        this->setOpacity(100);  // 变灰，不可用
        
        // 如果有背景图，也可以改变颜色来表示不可用状态
        if (_bg) {
            _bg->setColor(Color3B(128, 128, 128)); // 变灰
        }
        if (_icon) {
            _icon->setColor(Color3B(128, 128, 128)); // 图标也变灰
        }
    }
    else {
        this->setOpacity(255);  // 完全不透明，可用
        
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