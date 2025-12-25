// ʵ�������
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
    // ����������Ϊ "general/sun.png" ��ͼƬ�����û�У���ʱ�� Sprite::create() ��ʧ��
    // ������������һ���򵥵�ԲȦ���ף����������ز��ٻ� "general/sun.png"
    if (!Sprite::init()) return false;

    this->setTexture("general/sun.png"); 

    // ��ӵ������
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

    // �Զ���ת���� (����⿴������ת)
    this->runAction(RepeatForever::create(RotateBy::create(3.0f, 360)));

    return true;
}

void Sun::setOnCollectedCallback(const std::function<void(int)>& callback) {
    _onCollectedCallback = callback;
}

void Sun::fallFromSky(float startX, float targetY) {
    this->setPosition(startX, Director::getInstance()->getVisibleSize().height + 50); // ��Ļ�Ϸ�
    this->setVisible(true);

    // ���������� -> ͣ��һ��ʱ�� -> ��ʧ
    auto move = MoveTo::create(5.0f, Vec2(startX, targetY)); // 3�����ٵ���
    auto delay = DelayTime::create(3.0f); // ���ϴ�5��
    auto fadeOut = FadeOut::create(1.0f);
    auto remove = RemoveSelf::create();

    this->runAction(Sequence::create(move, delay, fadeOut, remove, nullptr));
}

void Sun::jumpFromPlant(Vec2 startPos, Vec2 targetPos) {
    this->setPosition(startPos);
    this->setScale(0.5f); // �ճ���Сһ��

    // ����������������
    ccBezierConfig bezier;
    bezier.controlPoint_1 = startPos + Vec2(0, 50); // ������
    bezier.controlPoint_2 = targetPos + Vec2(0, 80); // ��ߵ�
    bezier.endPosition = targetPos; // ���

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
    this->stopAllActions(); // ֹͣ�������ʧ����

    // �������������Ͻ� (UI �����λ�� 20, 680)
    // ע�⣺�����������ô� GameScene ����������ʱд��
    Vec2 targetUI = Vec2(40, Director::getInstance()->getVisibleSize().height - 40);

    auto move = MoveTo::create(0.5f, targetUI);
    auto scale = ScaleTo::create(0.5f, 0.3f); // ��С
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