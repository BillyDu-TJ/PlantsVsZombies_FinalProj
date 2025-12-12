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
    _sunLabel->setAnchorPoint(Vec2(0, 0.5f));
    _sunLabel->setPosition(Vec2(20, visibleSize.height - 40)); // 屏幕左上角，留点边距
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

    // --- 创建卡槽 (UI) ---
    std::vector<int> plantIds = { 1001, 1002 }; // 豌豆，向日葵

    float startX = 140.0f;
	float startY = visibleSize.height - 80.0f - 10.0f; // 卡牌高度 + 10.0f 边距
    float gapX = 70.0f;

    for (int id : plantIds) {
        auto card = SeedCard::create(id);
        card->setPosition(startX, startY);

        // 设置回调：点击卡片时，选中该植物，并显示 Ghost
        card->setOnSelectCallback([this](int plantId) {
            this->selectPlant(plantId);
            });

        this->addChild(card, 200); // UI 层级最高
        _seedCards.pushBack(card);

		startX += gapX;
    }

    // --- 创建 Ghost Sprite (初始隐藏) ---
    _ghostSprite = Sprite::create();
    _ghostSprite->setOpacity(128); // 半透明
    _ghostSprite->setVisible(false);
    this->addChild(_ghostSprite, 150); // 在植物之上，UI之下

    // --- 鼠标移动监听 (Desktop 平台) ---
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = CC_CALLBACK_1(GameScene::onMouseMove, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);


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

    // 4.子弹更新
    for (auto b : _bullets) {
        b->updateLogic(dt);
    }

    // 5.执行战斗判定
    updateCombatLogic();

    // 6.清理失效对象 (垃圾回收)
    // 移除死掉的僵尸
    for (auto it = _zombies.begin(); it != _zombies.end(); ) {
        if ((*it)->isDead()) {
            // 从 Scene 移除已经在 die() 里做了，这里只需从 Vector 移除
            it = _zombies.erase(it);
        }
        else {
            ++it;
        }
    }
    // 移除失效子弹
    for (auto it = _bullets.begin(); it != _bullets.end(); ) {
        if (!(*it)->isActive() || (*it)->getParent() == nullptr) {
            (*it)->removeFromParent(); // 确保从场景移除
            it = _bullets.erase(it);
        }
        else {
            ++it;
        }
    }

    // 7.UI 实时刷新
    // 更新阳光文字
    if (_sunLabel) {
        _sunLabel->setString("Sun: " + std::to_string(_currentSun));
    }

    // 更新卡片状态 (变亮/变灰)
    for (auto card : _seedCards) {
        // 让卡片自己判断：如果拥有阳光 < 卡片花费，就变半透明
        card->updateSunCheck(_currentSun);
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

        if (FileUtils::getInstance()->isFileExist(data.texturePath)) {
            _ghostSprite->setTexture(data.texturePath);
        }
        else {
            _ghostSprite->setTextureRect(Rect(0, 0, 60, 60)); // 兜底
        }
        _ghostSprite->setVisible(true);
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

        // 绑定射击回调
        plant->setOnShootCallback([this, row](Vec2 pos, int damage) {
            // 只有当该行有僵尸时才真的发射 (简单的 AI 优化)
            // 我们可以遍历一下 _zombies，看看有没有僵尸在当前行且在右边
            bool enemyInSight = false;
            for (auto z : this->_zombies) {
                CCLOG("[Info] Check: ZombieRow %d vs PlantRow %d", z->getRow(), row);

                if (z->getRow() == row && z->getPositionX() > pos.x && !z->isDead()) {
                    enemyInSight = true;
                    break;
                }
            }

            if (enemyInSight) {
                CCLOG("[Info] Enemy in sight! PEW PEW!");
                this->createBullet(pos, damage);
            }
            else {
                CCLOG("[Info] No enemy, holding fire.");
            }
            });

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

        for (auto card : _seedCards) {
            card->updateSunCheck(_currentSun);
        }

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

// 2. 实现 createBullet
void GameScene::createBullet(Vec2 startPos, int damage) {
    BulletData bData;
    bData.damage = damage;
    bData.speed = 400.0f;
    bData.texturePath = "bullets/pea.png"; // 确保你有这个图，或者代码里会画绿点

    auto bullet = Bullet::create(bData);
    bullet->setPosition(startPos);
    // 子弹的行号需要反算一下，或者直接不存行号单纯靠碰撞
    // 这里简单起见，我们假设子弹是矩形碰撞，不需要严格的 row 属性，
    // 但为了优化，如果给 Bullet 加 row 属性会更快。
    // 暂时先只设置 ZOrder
    this->addChild(bullet, 100);
    _bullets.pushBack(bullet);
}

// 实现核心战斗逻辑
void GameScene::updateCombatLogic() {
    // A. 子弹 vs 僵尸
    for (auto bullet : _bullets) {
        if (!bullet->isActive()) continue;

        // 优化：创建一个稍小的碰撞框，比图片原始 rect 小一点，体验更好
        Rect bRect = bullet->getBoundingBox();

        for (auto zombie : _zombies) {
            if (zombie->isDead()) continue;

            // 简单优化：如果子弹和僵尸Y轴差太多（跨行），直接跳过
            // 假设行高 100，允许误差 30
            if (std::abs(bullet->getPositionY() - zombie->getPositionY()) > 30) continue;

            if (bRect.intersectsRect(zombie->getBoundingBox())) {
                // 击中！
                zombie->takeDamage(bullet->getDamage());
                bullet->deactivate(); // 子弹消失
                bullet->removeFromParent();

                CCLOG("[Info] Bullet hit Zombie! Zombie HP: %d", zombie->getHp());
                break; // 一颗子弹只打一个
            }
        }
    }

    // B. 僵尸吃植物
    for (auto zombie : _zombies) {
        if (zombie->isDead()) continue;

        int row = zombie->getRow();
        float zombieMouthX = zombie->getPositionX() - 30;
        int col = (int)((zombieMouthX - GRID_START_X) / CELL_WIDTH);

        Plant* targetPlant = nullptr;
        // 检查当前格是否有效且有植物
        if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
            targetPlant = _plantMap[row][col];
        }

        if (targetPlant && !targetPlant->isDead()) {
            // 有植物 -> 吃
            if (zombie->getState() != UnitState::ATTACK) {
                zombie->setState(UnitState::ATTACK);
                CCLOG("[Info] Zombie starts eating plant at [%d, %d]", row, col);
            }
            
            // 询问僵尸是否可以攻击
            if (zombie->canAttack()) {
                // 造成真实的数值伤害
                int dmg = zombie->getDamage();
                targetPlant->takeDamage(dmg);

                // 重置僵尸的攻击 CD
                zombie->resetAttackTimer();

                CCLOG("[Info] Chomp! Plant HP: %d (Damage: %d)", targetPlant->getHp(), dmg);
            }

            if (targetPlant->isDead()) {
                CCLOG("[Info] Plant eaten by zombie!");
                _plantMap[row][col] = nullptr; // 植物死了，清空格子
                zombie->setState(UnitState::WALK); // 恢复行走
            }
        }
        else {
            // 没植物 -> 走
            if (zombie->getState() == UnitState::ATTACK) {
                zombie->setState(UnitState::WALK);
            }
        }
    }
}

// 鼠标移动回调
void GameScene::onMouseMove(Event* event) {
    EventMouse* e = (EventMouse*)event;
    // Cocos 的鼠标坐标原点在左下角，但 Y 轴有时需要转换，视版本而定
    // v4.0 通常是标准的 GL 坐标
    Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());

    if (_selectedPlantId != -1) {
        updateGhostPosition(mousePos);
    }
}

// 更新幽灵位置 (吸附网格)
void GameScene::updateGhostPosition(Vec2 mousePos) {
    auto grid = pixelToGrid(mousePos);
    int row = grid.first;
    int col = grid.second;

    if (row != -1) {
        // 在网格内，吸附到格子中心
        Vec2 snapPos = gridToPixel(row, col);
        _ghostSprite->setPosition(snapPos);
        _ghostSprite->setVisible(true);

        // 颜色提示：如果格子被占用或阳光不足，变红
        bool canPlant = (_plantMap[row][col] == nullptr);
        _ghostSprite->setColor(canPlant ? Color3B::WHITE : Color3B::RED);
    }
    else {
        // 在网格外，跟随鼠标 (或者隐藏)
        _ghostSprite->setPosition(mousePos);
        // _ghostSprite->setVisible(false); // 可选：出界隐藏
    }
}