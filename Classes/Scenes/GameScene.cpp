// 该文件实现了头文件中声明的 GameScene 类的方法。主要功能包括场景初始化、坐标转换和调试网格绘制。
// 2025.11.27 by BillyDu
#include <string> // C++11 string

#include "GameScene.h"
#include "../Consts.h" // 引用常量
#include "../Managers/DataManager.h"
#include "../Managers/LevelManager.h"
#include "../Managers/AudioManager.h"
#include "../Managers/SceneManager.h"  // 添加场景管理器头文件
#include "../Utils/GameException.h"
#include "../Entities/Plant.h"
#include "../Entities/Zombie.h"
#include "../Entities/Sun.h"
#include "ui/CocosGUI.h"  // 添加UI组件头文件

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

    // --- 初始化网格 ---
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            _plantMap[r][c] = nullptr;
        }
    }

    // --- 加载关卡 ---
    try {
        DataManager::getInstance().loadData(); // 确保数据先加载
        LevelManager::getInstance().loadLevel("data/level_test.json");
    }
    catch (const std::exception& e) {
        CCLOG("[Err] Init Error: %s", e.what());
    }

    // --- 开启 Update 调度 ---
    this->scheduleUpdate();

    const auto& assets = LevelManager::getInstance().getAssets();

    // [背景] 修改背景加载逻辑，避免错位问题
    if (FileUtils::getInstance()->isFileExist(assets.bgPath)) {
        auto bg = Sprite::create(assets.bgPath);
        
        // 计算合适的缩放比例，保持宽高比的同时填满屏幕
        Size bgSize = bg->getContentSize();
        float scaleX = visibleSize.width / bgSize.width;
        float scaleY = visibleSize.height / bgSize.height;
        float scale = std::max(scaleX, scaleY); // 使用较大的缩放比，确保完全覆盖屏幕
        
        bg->setScale(scale);
        bg->setPosition(visibleSize.width / 2, visibleSize.height / 2); // 居中显示
        bg->setAnchorPoint(Vec2(0.5f, 0.5f)); // 中心锚点
        
        this->addChild(bg, -1);
        
        CCLOG("[Info] Background loaded: %s, Original size: %.1fx%.1f, Scale: %.2f", 
              assets.bgPath.c_str(), bgSize.width, bgSize.height, scale);
    }
    else {
        auto bg = LayerColor::create(Color4B(0, 150, 0, 255));
        this->addChild(bg, -1);
        CCLOG("[Info] Using fallback green background");
    }

    // [网格草坪] 启用调试网格以验证对齐
    drawDebugGrid();
    
    // --- UI: 阳光栏与卡槽 (容器化) ---

    // [容器] 创建一个 Node 作为整个顶部 UI 的父节点
    // 这样以后要移动 UI，只动这个 Node 就行
    auto uiLayer = Node::create();
    uiLayer->setPosition(Vec2(20, visibleSize.height - 10)); // 定位到屏幕左上角
    uiLayer->setScale(0.8f);
    this->addChild(uiLayer, 1000);

    _sunLabel = Label::createWithTTF(std::to_string(_currentSun), "fonts/Marker Felt.ttf", 32); // 字体稍微改小一点适配缩放

    if (FileUtils::getInstance()->isFileExist(assets.sunBarPath)) {
        auto sunBar = Sprite::create(assets.sunBarPath);
        sunBar->setAnchorPoint(Vec2(0, 1)); // 左上角锚点
        sunBar->setPosition(0, 0);         // 相对 uiLayer (0,0)
        uiLayer->addChild(sunBar);

        // [修改 4: 文字对齐]
        // 你的素材是一个阳光图标下面带个卷轴。
        // 我们需要把文字放在卷轴的中心。
        // 假设阳光图标宽约 80px，卷轴在图标正下方。
        // 你需要微调下面这两个数字：
        float labelX = 55.0f; // 阳光图标的水平中心
        float labelY = 20.0f; // 卷轴的垂直中心 (相对于图片底部)

        _sunLabel->setPosition(labelX, labelY);
        _sunLabel->setAnchorPoint(Vec2(0.5f, 0.5f)); // 居中对齐
        _sunLabel->setColor(Color3B::BLACK);

        // 调试技巧：如果文字被图挡住了，设置 ZOrder
        sunBar->addChild(_sunLabel, 1);
    }
    else {
        // 兜底逻辑
        uiLayer->addChild(_sunLabel);
    }

    std::vector<int> plantIds = { 1001, 1002 };

    // [修改 5: 卡槽位置]
    // 因为 UI 整体缩小了，这里的坐标是相对于 uiLayer 的内部坐标
    // 假设阳光栏宽度大约是 85px，我们在它右边一点开始
    float startX = 120.0f;
    float startY = -110.0f; // 相对于 uiLayer 顶部向下 40px (垂直居中)
    float gapX = 80.0f;    // 卡片间距

    for (int id : plantIds) {
        auto card = SeedCard::create(id);

        // 把卡片加到 uiLayer，而不是加到 sunBar (层级更清晰)
        card->setPosition(startX, startY);
        // card->setScale(1.2f); // 卡片再微调大一点点

        card->setOnSelectCallback([this](int plantId) {
            this->selectPlant(plantId);
            });

        uiLayer->addChild(card);
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

    // 自然产出阳光调度器 (比如每 10 秒掉一个)
    this->schedule([this](float dt) {
        auto visibleSize = Director::getInstance()->getVisibleSize();

        // 随机 X 坐标 (在网格范围内)
        float randomX = GRID_START_X + (rand() % (int)(GRID_COLS * CELL_WIDTH));
        // 随机 Y 目标 (前半场)
        float randomY = GRID_START_Y + (rand() % (int)(GRID_ROWS * CELL_HEIGHT));

        auto sun = Sun::create();
        sun->fallFromSky(randomX, randomY);

        // 设置收集回调
        sun->setOnCollectedCallback([this](int value) {
            this->_currentSun += value;
            // 记得刷新 UI (update 里已经写了，这里其实不需要手动刷，但为了保险)
            });

        this->addChild(sun, 500); // 层级非常高，在 UI 下面，植物上面
        CCLOG("[Info] Sun falling from sky.");

        }, 10.0f, "sun_sky_scheduler");

    // 创建暂停按钮
    createPauseButton();

    return true;
}

void GameScene::update(float dt) {
    // 如果游戏不在进行状态，不更新逻辑
    if (_gameState != GameState::PLAYING) return;

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
        _sunLabel->setString(std::to_string(_currentSun));
    }

    // 更新卡片状态 (变亮/变灰)
    for (auto card : _seedCards) {
        // 让卡片自己判断：如果拥有阳光 < 卡片花费，就变半透明
        card->updateSunCheck(_currentSun);
    }
    
    // 8. 胜负判定
    checkVictoryCondition();
    checkGameOverCondition();
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

        // 5. 植物逻辑分流：

        if (plantData.type == "shooter") {
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
        }
        else if (plantData.type == "producer") {
            // --- 生产者逻辑 (产出阳光) ---
            plant->setOnShootCallback([this](Vec2 pos, int amount) {
                // 这里的 amount 参数没用，或者可以代表产出的阳光值

                // 创建阳光实体 (需要 #include "../Entities/Sun.h")
                auto sun = Sun::create();

                // 设定跳跃目标：从植物位置跳到稍微旁边一点的位置
                Vec2 targetPos = pos + Vec2(30, -30);
                sun->jumpFromPlant(pos, targetPos);

                // 设定收集回调：点中阳光加钱
                sun->setOnCollectedCallback([this](int value) {
                    this->_currentSun += value;
                    // 如果需要在收集瞬间刷新UI，可以在这里做，或者等下一帧 update
                    });

                // 阳光层级设高一点 (500)，保证盖住植物
                this->addChild(sun, 500);

                CCLOG("[Info] Sunflower produced a sun!");
                });
        }
        

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

        // 种植成功后播放音效
        AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);

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
    float x = _actualGridStartX + col * _actualCellWidth + _actualCellWidth / 2;
    float y = _actualGridStartY + row * _actualCellHeight + _actualCellHeight / 2;
    return Vec2(x, y);
}

std::pair<int, int> GameScene::pixelToGrid(cocos2d::Vec2 pos) {
    int col = (int)((pos.x - _actualGridStartX) / _actualCellWidth);
    int row = (int)((pos.y - _actualGridStartY) / _actualCellHeight);

    if (col < 0 || col >= GRID_COLS || row < 0 || row >= GRID_ROWS) {
        return { -1, -1 };
    }

    return { row, col };
}

void GameScene::drawDebugGrid() {
    auto drawNode = DrawNode::create();
    this->addChild(drawNode, 10); // Z-Order 设高一点，画在最上层

    // 画横线
    for (int i = 0; i <= GRID_ROWS; i++) {
        float y = _actualGridStartY + i * _actualCellHeight;
        float xStart = _actualGridStartX;
        float xEnd = _actualGridStartX + GRID_COLS * _actualCellWidth;
        drawNode->drawLine(Vec2(xStart, y), Vec2(xEnd, y), Color4F::WHITE);
    }

    // 画竖线
    for (int i = 0; i <= GRID_COLS; i++) {
        float x = _actualGridStartX + i * _actualCellWidth;
        float yStart = _actualGridStartY;
        float yEnd = _actualGridStartY + GRID_ROWS * _actualCellHeight;
        drawNode->drawLine(Vec2(x, yStart), Vec2(x, yEnd), Color4F::WHITE);
    }
    
    CCLOG("[Info] Debug grid drawn with dynamic parameters");
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

    // 播放射击音效
    AudioManager::getInstance().playEffect(AudioPath::SHOOT_SOUND);
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
                
                // 在僵尸死亡时播放音效
                if (zombie->isDead()) {
                    AudioManager::getInstance().playEffect(AudioPath::ZOMBIE_DIE_SOUND);
                }
                
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

void GameScene::createPauseButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    auto pauseButton = ui::Button::create();
    pauseButton->setTitleText("||");
    pauseButton->setTitleFontSize(32);
    pauseButton->setTitleColor(Color3B::WHITE);
    pauseButton->setColor(Color3B(0, 0, 0));
    pauseButton->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 50));
    pauseButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->onPauseButtonClicked(sender);
        }
    });
    this->addChild(pauseButton, 2000);
}

void GameScene::onPauseButtonClicked(cocos2d::Ref* sender) {
    if (_gameState == GameState::PLAYING) {
        pauseGame();
    } else if (_gameState == GameState::PAUSED) {
        resumeGame();
    }
}

void GameScene::pauseGame() {
    _gameState = GameState::PAUSED;
    Director::getInstance()->pause();
    AudioManager::getInstance().pauseBackgroundMusic();
    
    // 显示暂停提示
    auto pauseLabel = Label::createWithTTF("PAUSED", "fonts/Marker Felt.ttf", 64);
    pauseLabel->setPosition(Director::getInstance()->getVisibleSize().width/2, 
                          Director::getInstance()->getVisibleSize().height/2);
    pauseLabel->setColor(Color3B::YELLOW);
    pauseLabel->setTag(999); // 用于查找和删除
    this->addChild(pauseLabel, 3000);
}

void GameScene::resumeGame() {
    _gameState = GameState::PLAYING;
    Director::getInstance()->resume();
    AudioManager::getInstance().resumeBackgroundMusic();
    
    // 移除暂停提示
    this->removeChildByTag(999);
}

void GameScene::checkVictoryCondition() {
    // 检查是否所有波次完成且场上无僵尸
    if (LevelManager::getInstance().isAllWavesCompleted() && _zombies.empty()) {
        endGame(true);
    }
}

void GameScene::checkGameOverCondition() {
    // 检查是否有僵尸到达房屋（最左边）
    for (auto zombie : _zombies) {
        if (zombie->getPositionX() < GRID_START_X - 100) { // 到达房屋
            endGame(false);
            return;
        }
    }
}

void GameScene::endGame(bool isVictory) {
    if (_gameState != GameState::PLAYING) return; // 防止重复调用
    
    _gameState = isVictory ? GameState::VICTORY : GameState::GAME_OVER;
    
    // 延迟跳转，让玩家看到最终结果
    this->scheduleOnce([this, isVictory](float dt) {
        if (isVictory) {
            SceneManager::getInstance().gotoVictoryScene();
        } else {
            SceneManager::getInstance().gotoGameOverScene();
        }
    }, 2.0f, "end_game_delay");
    
    CCLOG("[Info] Game ended: %s", isVictory ? "Victory" : "Game Over");
}

void GameScene::calculateGridParameters(cocos2d::Sprite* background) {
    if (!background) return;
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Size bgOriginalSize = background->getContentSize();
    float bgScale = background->getScale();
    
    // 计算背景的实际显示尺寸
    Size bgActualSize = Size(bgOriginalSize.width * bgScale, bgOriginalSize.height * bgScale);
    
    // 根据背景实际尺寸调整网格参数
    // 假设原始背景设计分辨率为 1024x768，网格区域占背景的特定比例
    float bgWidthRatio = bgActualSize.width / 1024.0f;   // 根据你的背景图调整
    float bgHeightRatio = bgActualSize.height / 768.0f;  // 根据你的背景图调整
    
    _actualGridStartX = GRID_START_X * bgWidthRatio + (visibleSize.width - bgActualSize.width) / 2;
    _actualGridStartY = GRID_START_Y * bgHeightRatio + (visibleSize.height - bgActualSize.height) / 2;
    _actualCellWidth = CELL_WIDTH * bgWidthRatio;
    _actualCellHeight = CELL_HEIGHT * bgHeightRatio;
    
    CCLOG("[Info] Grid adjusted - Start: (%.1f, %.1f), Cell: (%.1f, %.1f)", 
          _actualGridStartX, _actualGridStartY, _actualCellWidth, _actualCellHeight);
}