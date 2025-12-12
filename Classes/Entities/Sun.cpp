// 实现阳光类
// 2025.12.12 by BillyDu
#include "Sun.h"

USING_NS_CC;

Sun* Sun::create() {
    Sun* ret = new (std::nothrow) Sun();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret; return nullptr;
}

bool Sun::init() {
    // 假设你有名为 "general/sun.png" 的图片，如果没有，暂时用 Sprite::create() 会失败
    // 这里我们先用一个简单的圆圈兜底，等你有了素材再换 "general/sun.png"
    if (!Sprite::init()) return false;

    this->setTexture("general/sun.png"); 

    // 添加点击监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this](Touch* t, Event* e) {
        if (_isCollected) return false;

        Vec2 p = this->getParent()->convertToNodeSpace(t->getLocation());
        if (this->getBoundingBox().containsPoint(p)) {
            this->collect();
            return true;
        }
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 自动旋转动画 (让阳光看起来在转)
    this->runAction(RepeatForever::create(RotateBy::create(3.0f, 360)));

    return true;
}

void Sun::setOnCollectedCallback(const std::function<void(int)>& callback) {
    _onCollectedCallback = callback;
}

void Sun::fallFromSky(float startX, float targetY) {
    this->setPosition(startX, Director::getInstance()->getVisibleSize().height + 50); // 屏幕上方
    this->setVisible(true);

    // 动作：掉落 -> 停留一段时间 -> 消失
    auto move = MoveTo::create(5.0f, Vec2(startX, targetY)); // 3秒慢速掉落
    auto delay = DelayTime::create(3.0f); // 地上呆5秒
    auto fadeOut = FadeOut::create(1.0f);
    auto remove = RemoveSelf::create();

    this->runAction(Sequence::create(move, delay, fadeOut, remove, nullptr));
}

void Sun::jumpFromPlant(Vec2 startPos, Vec2 targetPos) {
    this->setPosition(startPos);
    this->setScale(0.5f); // 刚出来小一点

    // 贝塞尔曲线抛物线
    ccBezierConfig bezier;
    bezier.controlPoint_1 = startPos + Vec2(0, 50); // 起跳点
    bezier.controlPoint_2 = targetPos + Vec2(0, 80); // 最高点
    bezier.endPosition = targetPos; // 落地

    auto jump = BezierTo::create(0.8f, bezier);
    auto scale = ScaleTo::create(0.5f, 1.0f);
    auto spawn = Spawn::create(jump, scale, nullptr);

    auto delay = DelayTime::create(5.0f);
    auto fadeOut = FadeOut::create(1.0f);
    auto remove = RemoveSelf::create();

    this->runAction(Sequence::create(spawn, delay, fadeOut, remove, nullptr));
}

void Sun::collect() {
    _isCollected = true;
    this->stopAllActions(); // 停止掉落或消失动作

    // 动作：飞向左上角 (UI 阳光栏位置 20, 680)
    // 注意：这里的坐标最好从 GameScene 传进来，暂时写死
    Vec2 targetUI = Vec2(40, Director::getInstance()->getVisibleSize().height - 40);

    auto move = MoveTo::create(0.5f, targetUI);
    auto scale = ScaleTo::create(0.5f, 0.3f); // 变小
    auto spawn = Spawn::create(move, scale, nullptr);

    auto callback = CallFunc::create([this]() {
        if (_onCollectedCallback) {
            _onCollectedCallback(_value);
        }
        CCLOG("[Info] Sun Collected! +%d", _value);
        this->removeFromParent();
        });

    this->runAction(Sequence::create(spawn, callback, nullptr));
}