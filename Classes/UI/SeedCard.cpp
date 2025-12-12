// 种植卡片类实现
// 2025.12.12 by BillyDu
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
    auto data = DataManager::getInstance().getPlantData(plantId);
    _cost = data.cost;

    // 2. 创建背景 (假设有个 card_bg.png，没有就画个框)
    // 这里我们简单画一个带颜色的 Layer 模拟卡片背景
    auto bg = LayerColor::create(Color4B(100, 100, 100, 200), 72, 96); // 宽72 高96
    this->addChild(bg, 0);
    this->setContentSize(Size(72, 96)); // 设置 Node 大小用于触摸检测

    // 3. 创建图标
    if (FileUtils::getInstance()->isFileExist(data.texturePath)) {
        _icon = Sprite::create(data.texturePath);
        // 缩放图标以适应卡片
        float scale = 50.0f / _icon->getContentSize().width;
        _icon->setScale(scale);
        _icon->setPosition(30, 50); // 居中偏上
        this->addChild(_icon, 1);
    }

    // 4. 创建消耗文字
    _costLabel = Label::createWithSystemFont(std::to_string(_cost), "Arial", 14);
    _costLabel->setPosition(30, 10); // 底部
    this->addChild(_costLabel, 1);

    // 5. 添加触摸监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true); // 吞噬事件，防止点穿到场景
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        // 转换坐标到 Node 空间
        Vec2 p = this->convertToNodeSpace(touch->getLocation());
        Rect rect = Rect(0, 0, this->getContentSize().width, this->getContentSize().height);

        if (rect.containsPoint(p)) {
            // 点击了卡片
            if (_onSelectCallback) {
                _onSelectCallback(_plantId);
            }
            // 简单的点击反馈：缩放一下
            this->runAction(Sequence::create(ScaleTo::create(0.1f, 0.9f), ScaleTo::create(0.1f, 1.0f), nullptr));
            return true;
        }
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void SeedCard::updateSunCheck(int currentSun) {
    this->setCascadeOpacityEnabled(true); // 开启级联，这样子节点也会变半透明
    // 如果阳光不足，变灰 (半透明)
    if (currentSun < _cost) {
        this->setOpacity(100);
    }
    else {
        this->setOpacity(255);
    }
}

void SeedCard::setOnSelectCallback(const std::function<void(int)>& callback) {
    _onSelectCallback = callback;
}