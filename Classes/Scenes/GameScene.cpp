// 该文件实现了头文件中声明的 GameScene 类的方法。主要功能包括场景初始化、坐标转换和调试网格绘制。
// 2025.11.27 by BillyDu
#include "GameScene.h"
#include "../Consts.h" // 引用常量

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 添加一个简单的背景色 (绿色草地)
    auto background = LayerColor::create(Color4B(0, 128, 0, 255));
    this->addChild(background, -1);

    // 2. 绘制调试网格 (关键！让你看到格子在哪里)
    drawDebugGrid();

    // 3. 开启 Update 调度
    this->scheduleUpdate();

    // 4. 添加鼠标/触摸点击监听 (测试 pixelToGrid 是否正确)
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        auto loc = touch->getLocation();
        auto gridPos = this->pixelToGrid(loc);

        CCLOG("Clicked at Screen: (%f, %f) -> Grid: [Row: %d, Col: %d]",
            loc.x, loc.y, gridPos.first, gridPos.second);

        return true;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void GameScene::update(float dt) {
    // 以后在这里处理僵尸移动、子弹飞行
}

/**
* @param row 行号 (0 开始)，从下到上增大，最下面为 0
* @param col 列号 (0 开始)，从左到右增大，最左边为 0
*/
cocos2d::Vec2 GameScene::gridToPixel(int row, int col) {
    // x = 起点 + 列号 * 宽 + 半宽 (居中)
    float x = GRID_START_X + col * CELL_WIDTH + CELL_WIDTH / 2;
    // y = 起点 + 行号 * 高 + 半高 (居中)
    float y = GRID_START_Y + row * CELL_HEIGHT + CELL_HEIGHT / 2;
    return Vec2(x, y);
}

std::pair<int, int> GameScene::pixelToGrid(cocos2d::Vec2 pos) {
    // 算法：(当前坐标 - 起点) / 格子大小
    int col = (int)((pos.x - GRID_START_X) / CELL_WIDTH);
    int row = (int)((pos.y - GRID_START_Y) / CELL_HEIGHT);

    // 边界检查
    if (col < 0 || col >= GRID_COLS || row < 0 || row >= GRID_ROWS) {
        return { -1, -1 }; // 无效位置
    }

    return { row, col };
}

void GameScene::drawDebugGrid() {
    auto drawNode = DrawNode::create();
    this->addChild(drawNode, 10); // Z-Order 设高一点，画在最上层

    // 画横线
    for (int i = 0; i <= GRID_ROWS; i++) {
        float y = GRID_START_Y + i * CELL_HEIGHT;
        float xStart = GRID_START_X;
        float xEnd = GRID_START_X + GRID_COLS * CELL_WIDTH;
        drawNode->drawLine(Vec2(xStart, y), Vec2(xEnd, y), Color4F::WHITE);
    }

    // 画竖线
    for (int i = 0; i <= GRID_COLS; i++) {
        float x = GRID_START_X + i * CELL_WIDTH;
        float yStart = GRID_START_Y;
        float yEnd = GRID_START_Y + GRID_ROWS * CELL_HEIGHT;
        drawNode->drawLine(Vec2(x, yStart), Vec2(x, yEnd), Color4F::WHITE);
    }
}