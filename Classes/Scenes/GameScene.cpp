// 该文件实现了头文件中声明的 GameScene 类的方法。主要功能包括场景初始化、坐标转换和调试网格绘制。
// 2025.11.27 by BillyDu
#include <string> // C++11 string

#include "GameScene.h"
#include "../Consts.h" // 引用常量
#include "../Managers/DataManager.h"
#include "../Managers/LevelManager.h"
#include "../Utils/GameException.h"
#include "../Entities/Plant.h"
#include "../Entities/Zombie.h"

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

    // 0. 初始化网格
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            _plantMap[r][c] = nullptr;
        }
    }

    // 1. 基础UI：显示阳光数
    _sunLabel = Label::createWithSystemFont("Sun: " + std::to_string(_currentSun), "Arial", 24);
    _sunLabel->setPosition(Vec2(100, 680)); // 左上角
    _sunLabel->setColor(Color3B::YELLOW);
    this->addChild(_sunLabel, 100);

    // 1. 添加一个简单的背景色 (绿色草地)
    auto background = LayerColor::create(Color4B(0, 128, 0, 255));
    this->addChild(background, -1);

    // 2. 绘制调试网格 (关键！让你看到格子在哪里)
    drawDebugGrid();

    // 3. 开启 Update 调度
    this->scheduleUpdate();


    // --- DataManager Test (Paradigm Showcase) ---
    try {
        // 1. 加载数据
        DataManager::getInstance().loadData();

        // 2. 尝试读取一个植物的数据 (豌豆射手 ID 1001)
        const auto& plantInfo = DataManager::getInstance().getPlantData(1001);

        CCLOG("[Info] TEST: Loaded Plant [%s], HP: %d, Cost: %d",
            plantInfo.name.c_str(), plantInfo.hp, plantInfo.cost);

        // 3. (可选) 故意测试一个不存在的 ID 来触发异常机制
        // DataManager::getInstance().getPlantData(9999); 

    }
    catch (const GameException& e) {
        // 优雅的异常处理：弹出错误框或返回主菜单
        CCLOG("[Err] Data Error: %s", e.what());
        // 实际上这里应该弹出一个 MessageBox
        return false;
    }
    catch (const std::exception& e) {
        CCLOG("[Err] Standard Error: %s", e.what());
        return false;
    }

    // --- 加载关卡 ---
    try {
        DataManager::getInstance().loadData(); // 确保数据先加载
        LevelManager::getInstance().loadLevel("data/level_test.json");
    }
    catch (const std::exception& e) {
        CCLOG("[Err] Init Error: %s", e.what());
    }

    this->scheduleUpdate(); // 确保 Update 开启

    //// --- 实体测试 ---
    //try {
    //    // 1. 创建一个豌豆射手 (ID 1001)
    //    auto plantData = DataManager::getInstance().getPlantData(1001);
    //    auto plant = Plant::createWithData(plantData);
    //    // 放在第 2 行，第 1 列
    //    auto pos = gridToPixel(2, 1);
    //    plant->setPosition(pos);
    //    plant->setRow(2);
    //    this->addChild(plant);
    //    CCLOG("[Info] Spawned plant at Row 2, Col 1");

    //    // 2. 创建一个僵尸
    //    auto zombie = Zombie::create();
    //    zombie->setSpeed(20.0f); // 走慢点
    //    // 放在第 2 行，屏幕最右侧
    //    auto zPos = gridToPixel(2, 8);
    //    zombie->setPosition(zPos);
    //    zombie->setRow(2);
    //    this->addChild(zombie);

    //    // 让僵尸动起来 (为了演示，直接 schedule updateLogic)
    //    // 实际上这些应该由 LevelManager 统一遍历调用
    //    zombie->schedule([zombie](float dt) {
    //        zombie->updateLogic(dt);
    //        }, "zombie_update");

    //}
    //catch (const std::exception& e) {
    //    CCLOG("[Err] Entity Spawn Error: %s", e.what());
    //}

    // --- 键盘监听 (用于切换植物：按1选豌豆，按2选向日葵) ---
    auto keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event* event) {
        if (code == EventKeyboard::KeyCode::KEY_1) {
            selectPlant(1001); // 假设 1001 是豌豆
        } else if (code == EventKeyboard::KeyCode::KEY_2) {
            selectPlant(1002); // 假设 1002 是向日葵
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyListener, this);


    // --- 鼠标监听 (实现种植) ---
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = [this](Touch* touch, Event* event) {
        auto loc = touch->getLocation();
        auto gridPos = this->pixelToGrid(loc);
        
        // 如果点击有效且在网格内
        if (gridPos.first != -1) {
            CCLOG("[Info] Clicked Grid: [%d, %d]", gridPos.first, gridPos.second);
            // 尝试种植
            this->tryPlantAt(gridPos.first, gridPos.second);
        }
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    return true;
}

void GameScene::update(float dt) {
    // 1. 让 LevelManager 检查是否刷怪
    // 使用 Lambda 表达式作为回调
    LevelManager::getInstance().update(dt, [this](int id, int row) {
        this->spawnZombie(id, row);
        });

    // 2. 更新所有僵尸的逻辑
    for (auto zombie : _zombies) {
        zombie->updateLogic(dt);
    }

    // 3. 更新所有植物逻辑
    for (auto plant : _plants) {
        plant->updateLogic(dt);
    }
}

void GameScene::spawnZombie(int id, int row) {
    try {
        // 1. [关键] 从 DataManager 获取僵尸配置
        const auto& zombieData = DataManager::getInstance().getZombieData(id);

        // 2. [关键] 使用带参数的 create 方法
        auto zombie = Zombie::createWithData(zombieData);

        // ... 后续位置设置代码保持不变 ...
        auto pixelPos = gridToPixel(row, GRID_COLS);
        pixelPos.x += 50.0f;

        zombie->setPosition(pixelPos);
        zombie->setRow(row);
        this->addChild(zombie, row * 10);

        _zombies.pushBack(zombie);

        CCLOG("[Info] Spawned Zombie [ID:%d] at Row:%d", id, row);
    }
    catch (const std::exception& e) {
        CCLOG("[Err] Failed to spawn zombie: %s", e.what());
    }
}

// 切换植物
void GameScene::selectPlant(int plantId) {
    try {
        // 验证ID是否存在，不存在会抛出异常
        auto data = DataManager::getInstance().getPlantData(plantId);
        _selectedPlantId = plantId;
        CCLOG("[Info] Selected Plant: %s (Cost: %d)", data.name.c_str(), data.cost);
    }
    catch (const std::exception& e) {
        CCLOG("[Err] Cannot select plant %d: %s", plantId, e.what());
    }
}

// 核心种植逻辑
void GameScene::tryPlantAt(int row, int col) {
	// 1. 检查格子是否已被占用， 若后续有可以种在植物上的植物，可在此处扩展逻辑
    if (_plantMap[row][col] != nullptr) {
        CCLOG("[Info] Grid [%d, %d] is already occupied!", row, col);
        return; // 种植失败
    }

    try {
        // 2. 获取当前选中植物的数据
        const auto& plantData = DataManager::getInstance().getPlantData(_selectedPlantId);

        // 3. 检查阳光是否足够
        if (_currentSun < plantData.cost) {
            CCLOG("[Info] Not enough sun! Have: %d, Need: %d", _currentSun, plantData.cost);
            return;
        }

        // 4. 生成植物对象
        auto plant = Plant::createWithData(plantData);

        // 设置位置
        auto pixelPos = gridToPixel(row, col);
        plant->setPosition(pixelPos);
        plant->setRow(row);

        // 添加到场景 (根据 Row 设置 ZOrder，防止遮挡关系错误)
        this->addChild(plant, row * 10 + 1);

        // 5. 更新游戏状态
        _plants.pushBack(plant);         // 加入容器
        _plantMap[row][col] = plant;     // 标记网格占用
        _currentSun -= plantData.cost;   // 扣除阳光

        // 6. 更新 UI
        if (_sunLabel) {
            _sunLabel->setString("Sun: " + std::to_string(_currentSun));
        }

        CCLOG("[Info] Successfully planted %s at [%d, %d]. Sun left: %d",
            plantData.name.c_str(), row, col, _currentSun);

    }
    catch (const std::exception& e) {
        CCLOG("[Err] Planting failed: %s", e.what());
    }
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