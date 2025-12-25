// 这个文件实际上是头文件，包含了 GameScene 类的方法，主要功能包括游戏的开始、暂停、转换和结束等设计。
// 2025.11.27 by BillyDu
// edited on 2025.12.21 by Zhao//the problems before are a lots...
// 更新：实现植物种植冷却系统，根据阳光值动态计算冷却时间，种植后启动冷却并更新卡片状态
// by Zhao.12.23
#include <string> // C++11 string
#include <unordered_map>
#include <set>
#include <climits>  // for INT_MAX

#include "GameScene.h"
#include "../Consts.h" // 游戏常量
#include "../Managers/DataManager.h"
#include "../Managers/LevelManager.h"
#include "../Managers/AudioManager.h"
#include "../Managers/SceneManager.h"  // 添加场景管理头文件
#include "../Utils/GameException.h"
#include "../Entities/Plant.h"
#include "../Entities/Zombie.h"
#include "../Entities/Sun.h"
#include "ui/CocosGUI.h"  // 添加UI控件头文件

USING_NS_CC;

// 全局：大嘴花（Chomper / Bigmouth）冷却计时表（秒）
static std::unordered_map<Plant*, float> g_bigmouthCooldowns;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // --- 根据地图ID确定实际网格行数和参数 ---
    int mapId = SceneManager::getInstance().getCurrentMapId();
    _actualGridRows = (mapId == 2 || mapId == 4) ? 6 : 5; // Map2/Map4有6行（含水池），Map1/Map3有5行
    
    // Map2/Map4需要调整网格参数：格子高度压缩到0.9倍，起始Y设为屏幕底部，让网格底部对齐屏幕底部
    if (mapId == 2 || mapId == 4) {
        _actualCellHeight = CELL_HEIGHT * 0.88f;  // 格子高度压缩到原来的0.88？这里不好给就这个value不要动了。。。
        // 网格底部对齐屏幕底部：起始Y = origin.y（屏幕底部），最下方位置不变
        _actualGridStartY = origin.y;
        CCLOG("[Info] Map %d: Using %d rows, cell height: %.1f (0.9x), start Y: %.1f (bottom aligned)",
              mapId, _actualGridRows, _actualCellHeight, _actualGridStartY);
    } else {
        // Map1/Map3保持原样
        _actualCellHeight = CELL_HEIGHT;
        _actualGridStartY = GRID_START_Y;
        CCLOG("[Info] Map %d: Using %d rows (default parameters)", mapId, _actualGridRows);
    }

    // --- 初始化网格数组（使用最大6行） ---
    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            _plantMap[r][c] = nullptr;
        }
    }

    // --- 加载数据与对应地图的关卡配置 ---
    try {
        DataManager::getInstance().loadData(); // 确保数据先加载

        std::string levelFile = "data/level_test.json";
        if (mapId == 2) {
            levelFile = "data/level_map2.json";
        } else if (mapId == 4) {
            levelFile = "data/level_map4.json";
    }
        LevelManager::getInstance().loadLevel(levelFile);
    }
    catch (const std::exception& e) {
        CCLOG("[Err] Init Error: %s", e.what());
        // ������ݼ���ʧ�ܣ�����false����ֹ������ʼ��
        return false;
    }

    // --- 绑定 Update 回调 ---
    this->scheduleUpdate();

    const auto& assets = LevelManager::getInstance().getAssets();

    // [动态] 修改背景加载逻辑，适配网格位置
    if (FileUtils::getInstance()->isFileExist(assets.bgPath)) {
        auto bg = Sprite::create(assets.bgPath);
        
        // 保持原始背景宽高比的同时填满屏幕
        Size bgSize = bg->getContentSize();
        float scaleX = visibleSize.width / bgSize.width;
        float scaleY = visibleSize.height / bgSize.height;
        float scale = std::max(scaleX, scaleY); // 使用较大缩放比，确保完全覆盖屏幕
        
        bg->setScale(scale);
        bg->setPosition(visibleSize.width / 2, visibleSize.height / 2); // ������ʾ
        bg->setAnchorPoint(Vec2(0.5f, 0.5f)); // 设置锚点
        
        this->addChild(bg, -1);
        
        CCLOG("[Info] Background loaded: %s, Original size: %.1fx%.1f, Scale: %.2f", 
              assets.bgPath.c_str(), bgSize.width, bgSize.height, scale);
    }
    else {
        auto bg = LayerColor::create(Color4B(0, 150, 0, 255));
        this->addChild(bg, -1);
        CCLOG("[Info] Using fallback green background");
    }

    // [调试网格] 绘制调试网格以确保正确
    // drawDebugGrid();
    
    // --- UI: 阳光显示与卡片 (左上方) ---

    // [动态] 创建一个 Node 作为容器 UI 的根节点
    // 这样以后要移动 UI，只需移动这个 Node 即可
    auto uiLayer = Node::create();
    uiLayer->setPosition(Vec2(20, visibleSize.height - 10)); // ��λ����Ļ���Ͻ�
    uiLayer->setScale(0.8f);
    this->addChild(uiLayer, 1000);

    _sunLabel = Label::createWithTTF(std::to_string(_currentSun), "fonts/Marker Felt.ttf", 32); // 调小一点字体避免遮挡

    if (FileUtils::getInstance()->isFileExist(assets.sunBarPath)) {
        auto sunBar = Sprite::create(assets.sunBarPath);
        sunBar->setAnchorPoint(Vec2(0, 1)); // ���Ͻ�ê��
        sunBar->setPosition(0, 0);         // 放在 uiLayer (0,0)
        uiLayer->addChild(sunBar);

        // [修改 4: 手动定位]
        // 加载资源后获得一个阳光条背景图，中心大约靠左。
        // 我们需要将数字放在阳光条右侧中心。
        // 假设阳光条宽约200px，高约80px，数字放在右侧中心
        // 需要微调数字的具体位置：
        float labelX = 55.0f; // 距离阳光条左侧的水平偏移
        float labelY = 20.0f; // 距离底部的垂直偏移（相对图片底部）

        _sunLabel->setPosition(labelX, labelY);
        _sunLabel->setAnchorPoint(Vec2(0.5f, 0.5f)); // 中心对齐
        _sunLabel->setColor(Color3B::BLACK);

        // 直接加在阳光条上，确保不会被其他东西遮挡
        sunBar->addChild(_sunLabel, 1);
    }
    else {
        // 回退逻辑
        uiLayer->addChild(_sunLabel);
    }

    // 从SceneManager获取选中的植物列表，如果没有则使用默认列表
    std::vector<int> plantIds = SceneManager::getInstance().getSelectedPlants();
    if (plantIds.empty()) {
        // 如果没有选中的植物，使用默认列表
        plantIds = { 1001, 1002, 1008 };
        CCLOG("[Info] No plants selected, using default plant list");
    }
    else {
        CCLOG("[Info] Using %zu selected plants", plantIds.size());
    }

    // [修改 5: 卡片位置]
    // 因为 UI 整体缩小了，所以我们需要调整卡片在 uiLayer 内部的位置
    // 现在阳光条宽度大约85px，卡片从阳光条右边一点开始
    float startX = 120.0f;
    float startY = -110.0f; // 相对于 uiLayer 往下 40px（垂直往下）
    float gapX = 80.0f;    // ��Ƭ���

    // �������ֵ��Χ��������ȴʱ����㣩
    _minCost = INT_MAX;
    _maxCost = 0;
    for (int id : plantIds) {
        try {
            auto data = DataManager::getInstance().getPlantData(id);
            if (data.cost < _minCost) _minCost = data.cost;
            if (data.cost > _maxCost) _maxCost = data.cost;
        } catch (const std::exception& e) {
            CCLOG("[Warn] Failed to get plant data for id %d: %s", id, e.what());
        }
    }
    
    // ���ֻ��һ��ֲ�������ֲ�����ֵ��ͬ������Ĭ�Ϸ�Χ
    if (_minCost == INT_MAX || _minCost == _maxCost) {
        _minCost = 0;
        _maxCost = 200; // Ĭ��������ֵ
    }
    
    CCLOG("[Info] Cost range: min=%d, max=%d", _minCost, _maxCost);

    for (int id : plantIds) {
        auto card = SeedCard::create(id);

        // 把卡片加到 uiLayer，而不是加到 sunBar（层级问题）
        card->setPosition(startX, startY);
        // card->setScale(1.2f); // 卡片稍微放大一点

        card->setOnSelectCallback([this](int plantId) {
            this->selectPlant(plantId);
            });

        uiLayer->addChild(card);
        _seedCards.pushBack(card);

        startX += gapX;
    }

    // --- 创建铲子UI ---
    // 计算最后一个卡片的位置，将铲子放在其右侧
    float lastCardX = startX - gapX; // 最后一个卡片的X位置
    float cardWidth = 72.0f; // 卡片宽度（从SeedCard的setContentSize得知）
    float shovelOffsetX = cardWidth / 2.0f + 20.0f; // 卡片右侧的偏移（卡片宽度的一半 + 间距）
    // 向右移动大约半个画面（注意uiLayer的缩放是0.8，所以需要除以0.8来补偿）
    float screenHalfWidth = visibleSize.width / 2.0f / 0.8f; // 转换为uiLayer的本地坐标
    createShovelUI(uiLayer, lastCardX + shovelOffsetX + screenHalfWidth, startY);

    // --- 创建 Ghost Sprite (种植预览) ---
    _ghostSprite = Sprite::create();
    _ghostSprite->setOpacity(128); // ��͸��
    _ghostSprite->setVisible(false);
    this->addChild(_ghostSprite, 150); // ��ֲ��֮�ϣ�UI֮��

    // --- 鼠标移动事件 (Desktop 平台) ---
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = CC_CALLBACK_1(GameScene::onMouseMove, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);


    // --- 触摸事件 (实现种植和挖取) ---
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = [this](Touch* touch, Event* event) {
        auto loc = touch->getLocation();
        
        // ����Ƿ����˲���
        if (_shovel && _shovelSlot) {
            Vec2 shovelWorldPos = _shovel->getParent()->convertToWorldSpace(_shovel->getPosition());
            Size shovelSize = _shovel->getContentSize();
            float shovelScale = _shovel->getScale();
            Rect shovelRect(
                shovelWorldPos.x - shovelSize.width * shovelScale * 0.5f,
                shovelWorldPos.y - shovelSize.height * shovelScale * 0.5f,
                shovelSize.width * shovelScale,
                shovelSize.height * shovelScale
            );
            
            if (shovelRect.containsPoint(loc)) {
                // ����˲��ӣ���ʼ�϶�
                _isShovelSelected = true;
                _isShovelDragging = true;
                _selectedPlantId = -1; // ȡ��ֲ��ѡ��
                // ����ghost sprite
                if (_ghostSprite) {
                    _ghostSprite->setVisible(false);
                }
                CCLOG("[Info] Shovel selected");
                return true;
            }
        }
        
        // ��������϶����ӣ�����Ƿ�����������
        if (_isShovelDragging) {
        auto gridPos = this->pixelToGrid(loc);
            if (gridPos.first != -1) {
                CCLOG("[Info] Shovel clicked Grid: [%d, %d]", gridPos.first, gridPos.second);
                this->tryDigAt(gridPos.first, gridPos.second);
                // ��ȡ�����ò���״̬
                resetShovel();
            }
            return true;
        }
        
        // 普通种植逻辑
        auto gridPos = this->pixelToGrid(loc);
        if (gridPos.first != -1) {
            CCLOG("[Info] Clicked Grid: [%d, %d]", gridPos.first, gridPos.second);
            // ������ֲ
            this->tryPlantAt(gridPos.first, gridPos.second);
        }
        return true;
    };
    
    touchListener->onTouchMoved = [this](Touch* touch, Event* event) {
        if (_isShovelDragging && _shovel && _shovel->getParent()) {
            // �϶����Ӹ������
            auto loc = touch->getLocation();
            // ����������ת��ΪuiLayer�ı�������
            Vec2 localPos = _shovel->getParent()->convertToNodeSpace(loc);
            _shovel->setPosition(localPos);
        }
        return true;
        };
    
    touchListener->onTouchEnded = [this](Touch* touch, Event* event) {
        if (_isShovelDragging) {
            auto loc = touch->getLocation();
            auto gridPos = this->pixelToGrid(loc);
            
            // ����ͷ��������ϣ�������ȡ
            if (gridPos.first != -1) {
                CCLOG("[Info] Shovel released on Grid: [%d, %d]", gridPos.first, gridPos.second);
                this->tryDigAt(gridPos.first, gridPos.second);
            }
            
            // ���ò���λ��
            resetShovel();
        }
        return true;
        };
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    // --- 键盘事件：ESC 暂停/继续 ---
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
            if (_gameState == GameState::PLAYING) {
                pauseGame();
            }
            else if (_gameState == GameState::PAUSED) {
                resumeGame();
            }
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    // 定时生成阳光（每隔 10 秒一个）
    this->schedule([this](float dt) {
        auto visibleSize = Director::getInstance()->getVisibleSize();

        // 随机 X 坐标（网格范围内）
        float randomX = GRID_START_X + (rand() % (int)(GRID_COLS * CELL_WIDTH));
        // 随机 Y 坐标（前中场，使用动态行数）
        float randomY = GRID_START_Y + (rand() % (int)(_actualGridRows * CELL_HEIGHT));

        auto sun = Sun::create();
        sun->fallFromSky(randomX, randomY);

        // 绑定收集回调
        sun->setOnCollectedCallback([this](int value) {
            this->_currentSun += value;
            // 记得刷新 UI（update 里已经写了，所以这里不需要手动刷新，但为了安全）
            });

        this->addChild(sun, 500); // 层级非常高，在 UI 上面，植物下面
        CCLOG("[Info] Sun falling from sky.");

        }, 10.0f, "sun_sky_scheduler");

    // ������ͣ��ť
    createPauseButton();

    return true;
}

void GameScene::update(float dt) {
    // 如果游戏不在进行状态，不执行逻辑
    if (_gameState != GameState::PLAYING) return;

    // 1. 向 LevelManager 查询是否刷新僵尸
    // ʹ�� Lambda ���ʽ��Ϊ�ص�
    LevelManager::getInstance().update(dt, [this](int id, int row) {
        this->spawnZombie(id, row);
        });

    // 2. 更新所有僵尸逻辑
    for (auto zombie : _zombies) {
        zombie->updateLogic(dt);
    }

    // 3. ��������ֲ���߼�
    for (auto plant : _plants) {
        plant->updateLogic(dt);
    }

    // 3.1 更新大嘴花冷却计时（秒），并清理已死亡/移除的植物
    for (auto it = g_bigmouthCooldowns.begin(); it != g_bigmouthCooldowns.end(); ) {
        Plant* plant = it->first;
        // 如果植物已经死亡或离场，直接移除冷却记录
        if (!plant || plant->isDead() || plant->getParent() == nullptr) {
            it = g_bigmouthCooldowns.erase(it);
            continue;
        }

        float& cd = it->second;
        if (cd > 0.0f) {
            cd -= dt;
            if (cd < 0.0f) cd = 0.0f;
        }
        ++it;
    }

    // 4. 子弹逻辑
    for (auto b : _bullets) {
        b->updateLogic(dt);
    }

    // 5. 执行战斗判断
    updateCombatLogic();

    // 6. 清理失效对象（死亡移除）
    // 移除死亡的僵尸
    for (auto it = _zombies.begin(); it != _zombies.end(); ) {
        if ((*it)->isDead()) {
            // 从 Scene 移除已经在 die() 里做了，这里只从 Vector 移除
            it = _zombies.erase(it);
        }
        else {
            ++it;
        }
    }
    // �Ƴ�ʧЧ�ӵ�
    for (auto it = _bullets.begin(); it != _bullets.end(); ) {
        if (!(*it)->isActive() || (*it)->getParent() == nullptr) {
            (*it)->removeFromParent(); // ȷ���ӳ����Ƴ�
            it = _bullets.erase(it);
        }
        else {
            ++it;
        }

    }

    // 7. UI 实时刷新
    // 刷新阳光显示
    if (_sunLabel) {
        _sunLabel->setString(std::to_string(_currentSun));
    }

    // 刷新卡片状态（可用/禁用）
    for (auto card : _seedCards) {
        // 让卡片自己判断：当前阳光 < 卡片花费，就变灰
        card->updateSunCheck(_currentSun);
        // ������ȴ״̬
        card->updateCooldown(dt);
    }
    
    // 8. 胜负判断
    checkVictoryCondition();
    checkGameOverCondition();
}

void GameScene::spawnZombie(int id, int row) {
    try {
        // 0. 根据地图与行号，决定是否需要替换为游泳僵尸（duck1 / duck2）
        int currentMapId = SceneManager::getInstance().getCurrentMapId();
        bool isWaterRow = (row == 2 || row == 3); // 第3、4行为水池
        int spawnId = id;

        // 在 Map2 和 Map4 的水池行，只生成游泳僵尸
        if (isWaterRow && (currentMapId == 2 || currentMapId == 4)) {
            // 普通 -> 游泳普通；锥桶 -> 游泳锥桶；其他全部按普通游泳处理
            if (id == 2002) {
                spawnId = 2007; // Conehead -> DuckConeheadZombie
            } else {
                spawnId = 2006; // 默认使用 DuckZombie
            }
        }

        // 1. [关键] 从 DataManager 获取僵尸数据
        const auto& baseData = DataManager::getInstance().getZombieData(spawnId);

        // 2. 根据当前地图（章节）应用难度系数（依次递增）
        ZombieData scaledData = baseData; // 拷贝一份可修改数据

        float hpMultiplier = 1.0f;
        float speedMultiplier = 1.0f;
        float damageMultiplier = 1.0f;

        // 简单难度曲线：第1章基础；之后每章提升
        switch (currentMapId) {
        case 1: // Day 1
            hpMultiplier = 1.0f;
            speedMultiplier = 1.0f;
            damageMultiplier = 1.0f;
            break;
        case 2: // Day 2
            hpMultiplier = 1.3f;
            speedMultiplier = 1.05f;
            damageMultiplier = 1.1f;
            break;
        case 3: // Night 1
            hpMultiplier = 1.6f;
            speedMultiplier = 1.1f;
            damageMultiplier = 1.2f;
            break;
        case 4: // Night 2
        default:
            hpMultiplier = 2.0f;
            speedMultiplier = 1.2f;
            damageMultiplier = 1.3f;
            break;
        }

        scaledData.hp = static_cast<int>(scaledData.hp * hpMultiplier);
        scaledData.speed = scaledData.speed * speedMultiplier;
        scaledData.damage = static_cast<int>(scaledData.damage * damageMultiplier);

        // 3. 验证行号是否在有效范围内（使用动态行数）
        if (row < 0 || row >= _actualGridRows) {
            CCLOG("[Warn] Invalid row %d for map %d (max rows: %d), skipping spawn", row, currentMapId, _actualGridRows);
            return;
        }

        // 4. [关键] 使用工厂方法 create 创建（使用缩放后的数据）
        auto zombie = Zombie::createWithData(scaledData);

        // ... 设置位置等代码保持不变 ...
        auto pixelPos = gridToPixel(row, GRID_COLS);
        pixelPos.x += 50.0f;

        zombie->setPosition(pixelPos);
        zombie->setRow(row);
        // 修复 Z-Order：越靠近屏幕底部的僵尸应该有更高的 Z-Order
        // 使用较大的基数减去行号，确保下方的僵尸在上层显示
        int zOrder = 1000 + row * 10;  // 行号越大，Z-Order 越高
        this->addChild(zombie, zOrder);

        _zombies.pushBack(zombie);

        CCLOG("[Info] Spawned Zombie [origID:%d -> finalID:%d] at Row:%d, MapId:%d (TotalRows:%d)",
              id, spawnId, row, currentMapId, _actualGridRows);
    }
    catch (const std::exception& e) {
        CCLOG("[Err] Failed to spawn zombie: %s", e.what());
    }
}

// 选择植物
void GameScene::selectPlant(int plantId) {
    try {
        // ����Ƿ�����ȴ��
        for (auto card : _seedCards) {
            if (card->getPlantId() == plantId && card->isInCooldown()) {
                CCLOG("[Info] Plant %d is in cooldown, cannot select", plantId);
                return; // ��ȴ�У�����ѡ��
            }
        }
        
        // ȡ������ѡ��
        if (_isShovelSelected || _isShovelDragging) {
            resetShovel();
        }
        
        // ��֤ID�Ƿ���ڣ������ڻ��׳��쳣
        auto data = DataManager::getInstance().getPlantData(plantId);
        _selectedPlantId = plantId;

        if (FileUtils::getInstance()->isFileExist(data.texturePath)) {
            _ghostSprite->setTexture(data.texturePath);
        }
        else {
            _ghostSprite->setTextureRect(Rect(0, 0, 60, 60)); // 占位
        }
        _ghostSprite->setVisible(true);
        CCLOG("[Info] Selected Plant: %s (Cost: %d)", data.name.c_str(), data.cost);
    }
    catch (const std::exception& e) {
        CCLOG("[Err] Cannot select plant %d: %s", plantId, e.what());
    }
}

// ������ֲ�߼�
void GameScene::tryPlantAt(int row, int col) {
    try {
        // 0. 检查是否在冷却中
        for (auto card : _seedCards) {
            if (card->getPlantId() == _selectedPlantId && card->isInCooldown()) {
                CCLOG("[Info] Plant %d is in cooldown, cannot plant", _selectedPlantId);
                return; // 冷却中，不能种植
            }
        }
        
        // 1. 获取当前选中植物的数据
        const auto& plantData = DataManager::getInstance().getPlantData(_selectedPlantId);
        
        // 2. 检查是否是水池行（Map2/Map4的第3、4行，即row 2和3）
        int currentMapId = SceneManager::getInstance().getCurrentMapId();
        bool isWaterRow = (currentMapId == 2 || currentMapId == 4) && (row == 2 || row == 3);
        bool isLilyPad = (plantData.name == "LilyPad");
        
        // 3. 水池种植逻辑检查
        if (isWaterRow) {
            Plant* existingPlant = _plantMap[row][col];
            
            if (isLilyPad) {
                // 在水池行种植睡莲：检查位置是否已被占用
                if (existingPlant != nullptr) {
                    CCLOG("[Info] Cannot plant LilyPad at [%d, %d]: position already occupied!", row, col);
                    return; // 种植失败
                }
            } else {
                // 在水池行种植非睡莲植物：必须先有睡莲
                if (existingPlant == nullptr) {
                    CCLOG("[Info] Cannot plant %s at water row [%d, %d]: need LilyPad first!", 
                          plantData.name.c_str(), row, col);
                    return; // 种植失败：没有睡莲
                }
                
                // 检查现有植物是否是睡莲
                if (existingPlant->getName() != "LilyPad") {
                    CCLOG("[Info] Cannot plant %s at water row [%d, %d]: position already has %s (not LilyPad)!", 
                          plantData.name.c_str(), row, col, existingPlant->getName().c_str());
                    return; // 种植失败：位置已有其他非睡莲植物
                }
                
                // 允许在睡莲上种植：睡莲保留，新植物叠加在上面
                // 注意：这里不删除睡莲，而是让新植物和睡莲共存
                CCLOG("[Info] Planting %s on LilyPad at [%d, %d]", plantData.name.c_str(), row, col);
            }
        } else {
            // 非水池行：检查位置是否已被占用
    if (_plantMap[row][col] != nullptr) {
        CCLOG("[Info] Grid [%d, %d] is already occupied!", row, col);
        return; // ��ֲʧ��
    }
        }

        // 4. 检查阳光是否足够
        if (_currentSun < plantData.cost) {
            CCLOG("[Info] Not enough sun! Have: %d, Need: %d", _currentSun, plantData.cost);
            return;
        }

        // 5. 创建植物对象
        auto plant = Plant::createWithData(plantData);

        // 6. 植物回调设置

        if (plantData.type == "shooter") {
            // 射击类植物
            // Capture plantId to determine if it's Repeater
            int currentPlantId = _selectedPlantId;
            plant->setOnShootCallback([this, row, currentPlantId](Vec2 pos, int damage) {
                // 只有当当前行有僵尸时才发射（简单的 AI 优化）
                // 我们可以遍历 _zombies，检查有没有僵尸在当前行且右侧
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

                    // Repeater (1008) һ�η��������㶹
                    if (currentPlantId == 1008) {
                        // �����һ���㶹
                        this->createBullet(pos, damage);
                        // ��΢�ӳٷ���ڶ����㶹����������΢���
                        this->runAction(Sequence::create(
                            DelayTime::create(0.05f),
                            CallFunc::create([this, pos, damage]() {
                                this->createBullet(pos, damage);
                                }),
                            nullptr
                        ));
                        CCLOG("[Info] Repeater shoots two peas!");
                    }
                    else if (currentPlantId == 1006) {
                        // SnowPea (1006) �������
                        this->createBullet(pos, damage, BulletType::ICE);
                        CCLOG("[Info] SnowPea shoots ice bullet!");
                    }
                    else if (currentPlantId == 1009) {
                        // PuffShroom (1009) ����Ģ���ӵ���������
                        this->createMushroomBullet(pos, damage);
                        CCLOG("[Info] PuffShroom shoots mushroom bullet!");
                    }
                    else if (currentPlantId == 1011) {
                        // FumeShroom (1011) ����Ģ���ӵ���������
                        this->createMushroomBullet(pos, damage);
                        CCLOG("[Info] FumeShroom shoots mushroom bullet!");
                    }
                    else {
                        // �������ֲ��ֻ����һ����ͨ�ӵ�
                    this->createBullet(pos, damage);
                }
                }
                else {
                    CCLOG("[Info] No enemy, holding fire.");
                }
                });
        }
        else if (plantData.type == "producer") {
            // --- 生产类逻辑（生产阳光） ---
            plant->setOnShootCallback([this](Vec2 pos, int amount) {
                // 这里的 amount 暂时没用，或者可以用来生成不同价值的阳光

                // 创建阳光实体（需要 #include "../Entities/Sun.h")
                auto sun = Sun::create();

                // 设定跳跃目标：植物位置右下方一点
                Vec2 targetPos = pos + Vec2(30, -30);
                sun->jumpFromPlant(pos, targetPos);

                // 设定收集回调：增加金钱
                sun->setOnCollectedCallback([this](int value) {
                    this->_currentSun += value;
                    // 如果需要立即刷新UI，可以在这里手动刷新，但一般等下一帧 update
                    });

                // 加一个高层级（500），确保盖住植物
                this->addChild(sun, 500);

                CCLOG("[Info] Sunflower produced a sun!");
                });
        }
        else if (plantData.type == "defensive") {
            // ������ֲ��
            if (plantData.name == "Spikeweed") {
                // �ش̣��Ծ�������Ľ�ʬÿ������˺�
                plant->setOnShootCallback([this, row, col](Vec2 pos, int damage) {
                    // ����ͬһ�еĽ�ʬ
                    for (auto z : this->_zombies) {
                        if (!z || z->isDead()) continue;
                        if (z->getRow() != row) continue;

                        // Boss2 ��ר�ŵ���ѹ�߼��д�������������ظ�����
                        if (z->isCrushingType()) continue;

                        float zombieX = z->getPositionX();
                        float cellLeft = _actualGridStartX + col * _actualCellWidth;
                        float cellRight = cellLeft + _actualCellWidth;

                        // ��ʬ�������ڵ�ǰ���ӷ�Χ�ڣ���Ϊ�������ش̡�
                        if (zombieX > cellLeft && zombieX < cellRight) {
                            z->takeDamage(damage);
                            CCLOG("[Info] Spikeweed damages zombie for %d at row %d, col %d", damage, row, col);
                        }
                    }
                });
        }
            // ��������ֲ��Ŀǰ����Ҫ�����߼����� TallNut ����������ֵ�赲��
        }
        

        // ����λ��
        auto pixelPos = gridToPixel(row, col);
        plant->setPosition(pixelPos);
        plant->setRow(row);

        int zOrder = 500 + row * 10;  // 植物基础 Z-Order 比僵尸低
        if (isWaterRow && !isLilyPad && _plantMap[row][col] != nullptr) {
            // 在睡莲上种植：新植物在睡莲之上
            zOrder = 500 + row * 10 + 2;
        }
        this->addChild(plant, zOrder);

        // 7. 更新游戏状态
        _plants.pushBack(plant);         // 加入列表
        
        // 如果是在睡莲上种植，睡莲保留在_plantMap中，新植物不覆盖它
        // 但我们需要记录新植物，以便后续逻辑能正确工作
        // 注意：_plantMap[row][col]仍然指向睡莲，新植物通过_plants列表管理
        if (isWaterRow && !isLilyPad) {
            // 在睡莲上种植：不覆盖_plantMap，睡莲和新植物共存
            // 战斗逻辑会遍历_plants列表，所以新植物也能正常工作
            CCLOG("[Info] Plant %s planted on LilyPad at [%d, %d] (LilyPad remains)", 
                  plantData.name.c_str(), row, col);
        } else {
            // 普通种植或种植睡莲：正常更新_plantMap
            _plantMap[row][col] = plant;
        }

        _currentSun -= plantData.cost;   // 扣除费用

        // 6. ���� UI
        if (_sunLabel) {
            _sunLabel->setString("Sun: " + std::to_string(_currentSun));
        }

        CCLOG("[Info] Successfully planted %s at [%d, %d]. Sun left: %d",
            plantData.name.c_str(), row, col, _currentSun);

        for (auto card : _seedCards) {
            card->updateSunCheck(_currentSun);
        }

        // �����ȴ
        for (auto card : _seedCards) {
            if (card->getPlantId() == _selectedPlantId) {
                float cooldownTime = calculateCooldownByCost(plantData.cost);
                card->startCooldown(cooldownTime);
                CCLOG("[Info] Started cooldown for plant %d: %.2f seconds", _selectedPlantId, cooldownTime);
                break;
            }
        }

        // ��ֲ�ɹ��󲥷���Ч
        AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);

        // ӣ��ը����CherryBomb�������߼�����ֲ��������ը
        if (plantData.name == "CherryBomb") {
            CCLOG("[Info] CherryBomb planted at [%d, %d], triggering immediate explosion!", row, col);
            
            // �ӳ�һ֡��ը��ȷ��ֲ���Ѿ���ӵ�������
            this->scheduleOnce([this, row, col, pixelPos](float dt) {
                // ��ȡֲ���˺�ֵ
                int damage = 5000; // CherryBomb�̶��˺�5000
                
                // ���ű�ը����
                this->createExplosionAnimation(pixelPos, "boom1", damage, row, col);
                
                // �Ƴ�ֲ��
                Plant* plant = _plantMap[row][col];
                if (plant) {
                    _plantMap[row][col] = nullptr;
                    _plants.eraseObject(plant);
                    plant->removeFromParent();
                    plant->release();
                    CCLOG("[Info] CherryBomb removed after explosion");
                }
            }, 0.1f, "cherrybomb_explode");
        }

    }
    catch (const std::exception& e) {
        CCLOG("[Err] Planting failed: %s", e.what());
    }
}

/**
* @param row 行号（0 开始），从上到下，最上行为 0
* @param col 列号（0 开始），从左到右，最左列为 0
*/
cocos2d::Vec2 GameScene::gridToPixel(int row, int col) {
    float x = _actualGridStartX + col * _actualCellWidth + _actualCellWidth / 2;
    float y = _actualGridStartY + row * _actualCellHeight + _actualCellHeight / 2;
    return Vec2(x, y);
}

std::pair<int, int> GameScene::pixelToGrid(cocos2d::Vec2 pos) {
    int col = (int)((pos.x - _actualGridStartX) / _actualCellWidth);
    int row = (int)((pos.y - _actualGridStartY) / _actualCellHeight);

    // 使用动态行数进行边界检查
    if (col < 0 || col >= GRID_COLS || row < 0 || row >= _actualGridRows) {
        return { -1, -1 };
    }

    return { row, col };
}

void GameScene::drawDebugGrid() {
    auto drawNode = DrawNode::create();
    this->addChild(drawNode, 10); // Z-Order 设高一点，放在最上层

    // 画横线（使用动态行数）
    for (int i = 0; i <= _actualGridRows; i++) {
        float y = _actualGridStartY + i * _actualCellHeight;
        float xStart = _actualGridStartX;
        float xEnd = _actualGridStartX + GRID_COLS * _actualCellWidth;
        drawNode->drawLine(Vec2(xStart, y), Vec2(xEnd, y), Color4F::WHITE);
    }

    // ������
    for (int i = 0; i <= GRID_COLS; i++) {
        float x = _actualGridStartX + i * _actualCellWidth;
        float yStart = _actualGridStartY;
        float yEnd = _actualGridStartY + _actualGridRows * _actualCellHeight;
        drawNode->drawLine(Vec2(x, yStart), Vec2(x, yEnd), Color4F::WHITE);
    }
    
    CCLOG("[Info] Debug grid drawn with %d rows (dynamic)", _actualGridRows);
}

// 2. ʵ�� createBullet
void GameScene::createBullet(Vec2 startPos, int damage, BulletType type, const std::string& texturePath) {
    BulletData bData;
    bData.damage = damage;
    bData.speed = 400.0f;
    bData.type = type;

    // Set texture path based on bullet type
    if (!texturePath.empty()) {
        bData.texturePath = texturePath;
    }
    else if (type == BulletType::ICE) {
        bData.texturePath = "bullets/PeaIce/PeaIce_0.png";
    }
    else {
        bData.texturePath = "bullets/pea.png";
    }

    // Set slow effect for ice bullets
    if (type == BulletType::ICE) {
        bData.slowEffect = 0.5f; // 50% speed
    }
    else {
        bData.slowEffect = 1.0f; // No slow effect
    } // 确保冰弹有正确的贴图和减速效果

    auto bullet = Bullet::create(bData);
    bullet->setPosition(startPos);
    // 子弹的行号暂时不需要设置，因为我们不是精确按行碰撞
    // 后面我们可以为子弹增加 row 属性来做更精确的碰撞
    // 临时方案：子弹是按全局碰撞的，不严格依赖 row 属性
    // 但因为优化需要，我们可以在 Bullet 类里增加 row 属性
    this->addChild(bullet, 100);
    _bullets.pushBack(bullet);

    // ���������Ч
    AudioManager::getInstance().playEffect(AudioPath::SHOOT_SOUND);
}

// 实现战斗逻辑
// Create mushroom bullet with animation
void GameScene::createMushroomBullet(Vec2 startPos, int damage) {
    BulletData bData;
    bData.damage = damage;
    bData.speed = 400.0f;
    bData.type = BulletType::NORMAL;
    bData.slowEffect = 1.0f; // No slow effect

    // Set up animation config for mushroom bullet
    bData.hasAnimation = true;
    bData.animationConfig.frameFormat = "bullets/BulletMushRoom/%d.png";
    bData.animationConfig.frameCount = 5; // 1.png to 5.png
    bData.animationConfig.frameDelay = 0.1f;
    bData.animationConfig.loopCount = -1; // Infinite loop
    bData.animationConfig.defaultTexture = "bullets/BulletMushRoom/1.png";

    // Fallback texture path
    bData.texturePath = "bullets/BulletMushRoom/1.png";

    auto bullet = Bullet::create(bData);
    bullet->setPosition(startPos);
    this->addChild(bullet, 100);
    _bullets.pushBack(bullet);

    // Play shoot sound effect
    AudioManager::getInstance().playEffect(AudioPath::SHOOT_SOUND);
}

// 创建爆炸动画
void GameScene::createExplosionAnimation(Vec2 pos, const std::string& boomType, int damage, int row, int col) {
    // boomType: "boom1" 用于CherryBomb, "boom2" 用于PotatoMine
    std::string frameFormat = "bullets/" + boomType + "/%d.png";
    
    CCLOG("[Info] Creating explosion animation: type=%s, pos=(%.1f, %.1f), damage=%d, row=%d, col=%d", 
          boomType.c_str(), pos.x, pos.y, damage, row, col);
    
    // 创建爆炸动画精灵
    auto explosionSprite = Sprite::create();
    explosionSprite->setPosition(pos);
    this->addChild(explosionSprite, 1000); // 最高层级，确保显示在最上层
    
    // 加载动画帧（自动检测帧数）
    Vector<SpriteFrame*> frames;
    float frameDelay = 0.08f;
    
    // 尝试加载最多30帧（足够覆盖大多数爆炸动画）
    bool hasFrames = false;
    int frameCount = 0;
    for (int i = 1; i <= 30; ++i) {
        char framePath[256];
        snprintf(framePath, sizeof(framePath), frameFormat.c_str(), i);
        
        if (FileUtils::getInstance()->isFileExist(framePath)) {
            // 先加载纹理，然后创建SpriteFrame
            Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(framePath);
            if (texture) {
                Size textureSize = texture->getContentSize();
                Rect rect = Rect(0, 0, textureSize.width, textureSize.height);
                SpriteFrame* frame = SpriteFrame::createWithTexture(texture, rect);
                if (frame) {
                    frames.pushBack(frame);
                    hasFrames = true;
                    frameCount++;
                    CCLOG("[Debug] Loaded explosion frame %d: %s", i, framePath);
                } else {
                    CCLOG("[Warn] Failed to create SpriteFrame from texture: %s", framePath);
                }
            } else {
                CCLOG("[Warn] Failed to load texture: %s", framePath);
            }
        } else {
            // 如果当前帧不存在，且已经有帧了，说明动画结束
            if (hasFrames) {
                CCLOG("[Info] Explosion animation ends at frame %d (total %d frames loaded)", i - 1, frameCount);
                break;
            }
        }
    }
    
    if (!hasFrames) {
        CCLOG("[Err] No explosion frames found for %s (format: %s), using placeholder", boomType.c_str(), frameFormat.c_str());
        // 使用占位符
        explosionSprite->setTextureRect(Rect(0, 0, 100, 100));
        explosionSprite->setColor(Color3B::RED);
    } else {
        CCLOG("[Info] Created explosion animation with %d frames for %s", frameCount, boomType.c_str());
        // 创建动画
        auto animation = Animation::createWithSpriteFrames(frames, frameDelay);
        auto animate = Animate::create(animation);
        
        // 设置初始纹理为第一帧
        if (!frames.empty()) {
            explosionSprite->setSpriteFrame(frames.at(0));
        }
        
        // 播放动画后移除
        explosionSprite->runAction(Sequence::create(
            animate,
            CallFunc::create([explosionSprite]() {
                explosionSprite->removeFromParent();
            }),
            nullptr
        ));
    }
    
    // 对周围僵尸造成伤害
    if (boomType == "boom1") {
        // CherryBomb: 炸死周围8个格子的僵尸（3x3范围，中心+周围8个）
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                int checkRow = row + dr;
                int checkCol = col + dc;
                
                if (checkRow >= 0 && checkRow < _actualGridRows && 
                    checkCol >= 0 && checkCol < GRID_COLS) {
                    // 检查该位置的僵尸
                    for (auto zombie : _zombies) {
                        if (zombie->isDead()) continue;
                        if (zombie->getRow() != checkRow) continue;
                        
                        // 检查僵尸是否在这个格子内
                        Vec2 zombiePos = zombie->getPosition();
                        Vec2 cellPos = gridToPixel(checkRow, checkCol);
                        float distX = std::abs(zombiePos.x - cellPos.x);
                        float distY = std::abs(zombiePos.y - cellPos.y);
                        
                        if (distX < _actualCellWidth / 2 && distY < _actualCellHeight / 2) {
                            zombie->takeDamage(damage);
                            CCLOG("[Info] CherryBomb explosion damages zombie at [%d, %d] for %d", 
                                  checkRow, checkCol, damage);
                            if (zombie->isDead()) {
                                AudioManager::getInstance().playEffect(AudioPath::ZOMBIE_DIE_SOUND);
                            }
                        }
                    }
                }
            }
        }
    } else {
        // PotatoMine: 炸死同一行的所有僵尸（确保能炸到，扩大范围）
        // 检查当前格及前后两格（共5格范围），确保能覆盖到所有可能的僵尸位置
        for (int offset = -2; offset <= 2; ++offset) {
            int checkCol = col + offset;
            if (checkCol < 0 || checkCol >= GRID_COLS) continue;
            
            Vec2 checkCellPos = gridToPixel(row, checkCol);
            
            for (auto zombie : _zombies) {
                if (zombie->isDead()) continue;
                if (zombie->getRow() != row) continue;
                
                // 检查僵尸是否在这个格子范围内（使用很大的检测范围）
                Vec2 zombiePos = zombie->getPosition();
                float distX = std::abs(zombiePos.x - checkCellPos.x);
                float distY = std::abs(zombiePos.y - checkCellPos.y);
                
                // 使用非常大的检测范围（1.5倍格子大小），确保一定能炸到僵尸
                if (distX < _actualCellWidth * 1.5f && distY < _actualCellHeight * 1.5f) {
                    int zombieHpBefore = zombie->getHp();
                    zombie->takeDamage(damage);
                    int zombieHpAfter = zombie->getHp();
                    CCLOG("[Info] PotatoMine explosion damages zombie at [%d, %d] (checkCol: %d, offset: %d) for %d (HP: %d -> %d)", 
                          row, col, checkCol, offset, damage, zombieHpBefore, zombieHpAfter);
                    if (zombie->isDead()) {
                        CCLOG("[Info] Zombie killed by PotatoMine explosion!");
                        AudioManager::getInstance().playEffect(AudioPath::ZOMBIE_DIE_SOUND);
                    }
                }
            }
        }
    }
}

void GameScene::updateCombatLogic() {
    // A. �ӵ� vs ��ʬ
    for (auto bullet : _bullets) {
        if (!bullet->isActive()) continue;

        // 优化：使用一个稍小的碰撞框，比图片原始 rect 小一点，避免误判
        Rect bRect = bullet->getBoundingBox();

        for (auto zombie : _zombies) {
            if (zombie->isDead()) continue;

            // 简单优化：如果子弹和僵尸Y轴相差太多（行不同），直接跳过
            // 这里的 100 是预估行高，实际可能是 30
            if (std::abs(bullet->getPositionY() - zombie->getPositionY()) > 30) continue;

            if (bRect.intersectsRect(zombie->getBoundingBox())) {
                // ���У�
                zombie->takeDamage(bullet->getDamage());

                // Apply slow effect if it's an ice bullet
                if (bullet->getType() == BulletType::ICE) {
                    CCLOG("[Info] Ice bullet hit zombie! Applying slow effect: %.1f%%", bullet->getSlowEffect() * 100.0f);
                    zombie->applySlowEffect(bullet->getSlowEffect());
                }
                else {
                    CCLOG("[Info] Normal bullet hit zombie (no slow effect)");
                }

                bullet->deactivate(); // �ӵ���ʧ
                bullet->removeFromParent();

                CCLOG("[Info] Bullet hit Zombie! Zombie HP: %d", zombie->getHp());
                
                // �ڽ�ʬ����ʱ������Ч
                if (zombie->isDead()) {
                    AudioManager::getInstance().playEffect(AudioPath::ZOMBIE_DIE_SOUND);
                }
                
                break; // һ���ӵ�ֻ��һ��
            }
        }
    }

    // B. 僵尸吃植物 / Boss2碾压植物
    for (auto zombie : _zombies) {
        if (zombie->isDead()) continue;

        int row = zombie->getRow();
        
        // Boss2 (snow sled) crushes plants directly without stopping
        if (zombie->isCrushingType()) {
            // Check all grid cells that Boss2 is passing through
            float zombieX = zombie->getPositionX();
            float zombieLeft = zombieX - 50;  // Approximate left edge
            float zombieRight = zombieX + 50;  // Approximate right edge
            
            // Check each column that Boss2 covers
            for (int col = 0; col < GRID_COLS; col++) {
                float cellLeft = _actualGridStartX + col * _actualCellWidth;
                float cellRight = cellLeft + _actualCellWidth;
                
                // If Boss2 overlaps with this cell
                if (zombieRight > cellLeft && zombieLeft < cellRight) {
                    Plant* plant = _plantMap[row][col];
                    
                    // ���_plantMap����˯��������λ���Ƿ�������ֲ���˯������ֲ�ģ�
                    if (plant && plant->getName() == "LilyPad") {
                        // ����_plants�б������ͬһλ�õķ�˯��ֲ��
                        for (auto p : _plants) {
                            if (!p || p->isDead() || p == plant) continue;
                            if (p->getRow() == row) {
                                // ����Ƿ���ͬһ�У�ͨ��λ���жϣ�
                                auto pPos = p->getPosition();
                                auto targetPos = gridToPixel(row, col);
                                float distX = std::abs(pPos.x - targetPos.x);
                                if (distX < _actualCellWidth / 2) {
                                    // �ҵ�˯���ϵ�ֲ�������ѹ��
                                    plant = p;
                                    break;
                                }
                            }
                        }
                    }
                    
                    if (plant && !plant->isDead()) {
                        // �ش̶� Boss2��һ����� 2000 �˺���ͬʱ����ѹ����
                        if (plant->getName() == "Spikeweed") {
                            CCLOG("[Info] Boss2 runs over Spikeweed at [%d, %d]! Spikeweed deals 2000 damage and is crushed.", row, col);
                            zombie->takeDamage(2000);
                            plant->takeDamage(9999); // �ش̱���ѹ
                            if (plant->isDead()) {
                                // ����Ƿ���˯���ϵĵش�
                                Plant* lilyPad = _plantMap[row][col];
                                if (lilyPad && lilyPad->getName() == "LilyPad" && plant != lilyPad) {
                                    CCLOG("[Info] Spikeweed on LilyPad crushed, LilyPad remains");
                                } else {
                                    _plantMap[row][col] = nullptr;
                                }
                            }
                        } else if (plant->getName() == "PotatoMine") {
                            // �����ף�Boss2��ѹʱ������ը
                            CCLOG("[Info] Boss2 triggers PotatoMine at [%d, %d]!", row, col);
                            int damage = plant->getData().attack;
                            if (damage <= 0) damage = 5000;
                            Vec2 plantPos = plant->getPosition();
                            
                            // ���ű�ը����
                            createExplosionAnimation(plantPos, "boom2", damage, row, col);
                            
                            // ���ֲ��Ϊ�������ӳ��Ƴ��������ڱ���ʱ�޸ļ��ϣ�
                            plant->takeDamage(9999);
                            _plantMap[row][col] = nullptr;
                            
                            // �ӳ��Ƴ��������ڱ���ʱ�޸ļ���
                            Plant* plantToRemove = plant;
                            this->scheduleOnce([this, plantToRemove](float dt) {
                                if (plantToRemove && plantToRemove->getParent()) {
                                    _plants.eraseObject(plantToRemove);
                                    plantToRemove->removeFromParent();
                                }
                            }, 0.01f, "remove_potatomine_boss2");
                        } else {
                            // ����ֲ�ֱ����ѹ����
                            CCLOG("[Info] Boss2 crushed plant at [%d, %d]!", row, col);
                            plant->takeDamage(9999);
                            if (plant->isDead()) {
                                // ����Ƿ���˯���ϵ�ֲ��
                                Plant* lilyPad = _plantMap[row][col];
                                if (lilyPad && lilyPad->getName() == "LilyPad" && plant != lilyPad) {
                                    // ˯���ϵ�ֲ�ﱻ��ѹ��˯������
                                    CCLOG("[Info] Plant on LilyPad crushed, LilyPad remains");
                                } else {
                                    // ��ֲͨ���˯������ѹ�����_plantMap
                                    _plantMap[row][col] = nullptr;
                                }
                            }
                        }
                    }
                }
            }
            
            // Place ice effect behind Boss2 (one ice per grid cell)
            // Calculate which grid cell Boss2 is currently in
            float currentX = zombie->getPositionX();
            int currentCol = (int)((currentX - _actualGridStartX) / _actualCellWidth);
            
            if (currentCol >= 0 && currentCol < GRID_COLS) {
                // Check if ice already exists at this grid position
                std::pair<int, int> iceKey = std::make_pair(row, currentCol);
                if (_icePositions.find(iceKey) == _icePositions.end()) {
                    // Place ice at the center of this grid cell
                    Vec2 icePos = gridToPixel(row, currentCol);
                    icePos.x += _actualCellWidth / 2;
                    
                    auto iceSprite = Sprite::create("zombies/boss2/ice.png");
                    if (iceSprite) {
                        iceSprite->setPosition(icePos);
                        iceSprite->setTag(9999);  // Tag to identify ice sprites
                        iceSprite->setLocalZOrder(-1);  // Behind everything
                        this->addChild(iceSprite);
                        _icePositions.insert(iceKey);  // Mark this position as having ice
                        CCLOG("[Info] Boss2 placed ice at grid [%d, %d]", row, currentCol);
                    }
                }
            }
            
            continue;  // Skip normal attack logic for Boss2
        }

        // Normal zombie attack logic
        float zombieMouthX = zombie->getPositionX() - 30;
        int col = (int)((zombieMouthX - _actualGridStartX) / _actualCellWidth);

        Plant* targetPlant = nullptr;
        // 检查当前格是否有有效的植物
        // 优先检查_plantMap（可能是睡莲或其他植物）
        if (col >= 0 && col < GRID_COLS && row >= 0 && row < _actualGridRows) {
            targetPlant = _plantMap[row][col];
            
            // ���_plantMap����˯��������λ���Ƿ�������ֲ���˯������ֲ�ģ�
            if (targetPlant && targetPlant->getName() == "LilyPad") {
                // ����_plants�б������ͬһλ�õķ�˯��ֲ��
                for (auto p : _plants) {
                    if (!p || p->isDead() || p == targetPlant) continue;
                    if (p->getRow() == row) {
                        // ����Ƿ���ͬһ�У�ͨ��λ���жϣ�
                        auto pPos = p->getPosition();
                        auto targetPos = gridToPixel(row, col);
                        float distX = std::abs(pPos.x - targetPos.x);
                        if (distX < _actualCellWidth / 2) {
                            // �ҵ�˯���ϵ�ֲ����ȹ�����
                            targetPlant = p;
                            break;
                        }
                    }
                }
            }
        }

        // �ش̣�Spikeweed�����ᱻ��ʬ����������Ҳ�����赲��ʬǰ��
        if (targetPlant && targetPlant->getName() == "Spikeweed") {
            targetPlant = nullptr;
        }

        // ���컨��Chomper / Bigmouth�����⹥���߼���ֱ���̵���ͨ��ʬ��30 ����ȴ
        if (targetPlant && targetPlant->getName() == "Chomper") {
            // Boss2 ��Ч���Ѿ��� isCrushingType ��֧�й��ˣ�����ֻ������ͨ��ʬ
            float currentCd = 0.0f;
            auto cdIt = g_bigmouthCooldowns.find(targetPlant);
            if (cdIt != g_bigmouthCooldowns.end()) {
                currentCd = cdIt->second;
        }

            if (currentCd <= 0.0f) {
                // ���컨�������������ųԶ�����ֱ�ӡ��Ե����ʬ
                targetPlant->playAnimation("eat");
                int zombieHp = zombie->getHp();
                zombie->takeDamage(zombieHp > 0 ? zombieHp : 9999);
                CCLOG("[Info] Chomper at [%d, %d] ate a zombie! Starting 30s cooldown.", row, col);

                // ���� 30 ����ȴ
                g_bigmouthCooldowns[targetPlant] = 30.0f;

                // ��ʬ���Ե�󣬵�ǰѭ�����ٶ��������ͨ�����߼�
                continue;
            }
            // ��ȴ�У����컨�޷���������ʬ����ҧ��
        }

        if (targetPlant && !targetPlant->isDead()) {
            // ��ֲ�� -> ��
            if (zombie->getState() != UnitState::ATTACK) {
                zombie->setState(UnitState::ATTACK);
                CCLOG("[Info] Zombie starts eating plant at [%d, %d]", row, col);
            }
            
            // ѯ�ʽ�ʬ�Ƿ���Թ���
            if (zombie->canAttack()) {
                // 土豆雷（PotatoMine）特殊逻辑：当僵尸开始eat时立即爆炸
                if (targetPlant->getName() == "PotatoMine" && !targetPlant->isDead()) {
                    CCLOG("[Info] PotatoMine at [%d, %d] triggered by zombie eating!", row, col);
                    
                    // 获取植物伤害值
                    int damage = targetPlant->getData().attack;
                    if (damage <= 0) damage = 5000;
                    
                    Vec2 plantPos = targetPlant->getPosition();
                    
                    // 播放爆炸动画
                    createExplosionAnimation(plantPos, "boom2", damage, row, col);
                    
                    // 找到所有同一行的僵尸并杀死它们（扩大范围，确保能炸到）
                    // 检查当前格及前后两格（共5格范围），确保能覆盖到所有可能的僵尸位置
                    for (int offset = -2; offset <= 2; ++offset) {
                        int checkCol = col + offset;
                        if (checkCol < 0 || checkCol >= GRID_COLS) continue;
                        
                        Vec2 checkCellPos = gridToPixel(row, checkCol);
                        
                        for (auto z : _zombies) {
                            if (z->isDead()) continue;
                            if (z->getRow() != row) continue;
                            
                            // 检查僵尸是否在这个格子范围内（使用很大的检测范围）
                            Vec2 zombiePos = z->getPosition();
                            float distX = std::abs(zombiePos.x - checkCellPos.x);
                            float distY = std::abs(zombiePos.y - checkCellPos.y);
                            
                            // 使用非常大的检测范围（1.5倍格子大小），确保一定能炸到僵尸
                            if (distX < _actualCellWidth * 1.5f && distY < _actualCellHeight * 1.5f) {
                                // 这个僵尸在爆炸范围内，杀死它
                                int zombieHp = z->getHp();
                                z->takeDamage(damage);
                                CCLOG("[Info] Zombie at [%d, %d] (checkCol: %d, offset: %d) killed by PotatoMine explosion! (HP: %d -> %d)", 
                                      row, col, checkCol, offset, zombieHp, z->getHp());
                                if (z->isDead()) {
                                    AudioManager::getInstance().playEffect(AudioPath::ZOMBIE_DIE_SOUND);
                                }
                            }
                        }
                    }
                    
                    // 标记植物为死亡，延迟移除（避免在遍历时修改集合）
                    targetPlant->takeDamage(9999);
                    _plantMap[row][col] = nullptr;
                    
                    // 延迟移除，避免在遍历时修改集合
                    Plant* plantToRemove = targetPlant;
                    this->scheduleOnce([this, plantToRemove](float dt) {
                        if (plantToRemove && plantToRemove->getParent()) {
                            _plants.eraseObject(plantToRemove);
                            plantToRemove->removeFromParent();
                        }
                    }, 0.01f, "remove_potatomine_eat");
                    
                    // 跳过后续攻击逻辑
                    continue;
                }
                
                // 这里应该使用僵尸的实际伤害值
                int dmg = zombie->getDamage();
                targetPlant->takeDamage(dmg);

                // ���ý�ʬ�Ĺ��� CD
                zombie->resetAttackTimer();

                CCLOG("[Info] Chomp! Plant HP: %d (Damage: %d)", targetPlant->getHp(), dmg);
            }

            if (targetPlant->isDead()) {
                CCLOG("[Info] Plant eaten by zombie!");
                
                // 检查是否是睡莲上的植物
                Plant* lilyPad = _plantMap[row][col];
                if (lilyPad && lilyPad->getName() == "LilyPad" && targetPlant != lilyPad) {
                    // 睡莲上的植物死亡，只移除该植物，睡莲保留
                    CCLOG("[Info] Plant on LilyPad died, LilyPad remains at [%d, %d]", row, col);
                } else {
                    // 普通植物或睡莲死亡，清空_plantMap
                    _plantMap[row][col] = nullptr;
                }
                
                zombie->setState(UnitState::WALK); // �ָ�����
            }
        }
        else {
            // ûֲ�� -> ��
            if (zombie->getState() == UnitState::ATTACK) {
                zombie->setState(UnitState::WALK);
            }
        }
    }
}

// ����ƶ��ص�
void GameScene::onMouseMove(Event* event) {
    EventMouse* e = (EventMouse*)event;
    // Cocos 坐标原点在左下角，但 Y 轴可能需要转换，根据版本
    // v4.0 通常是标准的 GL 坐标系
    Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());

    if (_selectedPlantId != -1) {
        updateGhostPosition(mousePos);
    }
}

// 更新幽灵位置（跟随鼠标）
void GameScene::updateGhostPosition(Vec2 mousePos) {
    auto grid = pixelToGrid(mousePos);
    int row = grid.first;
    int col = grid.second;

    if (row != -1) {
        // 网格内，就吸附到网格中心
        Vec2 snapPos = gridToPixel(row, col);
        _ghostSprite->setPosition(snapPos);
        _ghostSprite->setVisible(true);

        // 颜色提示：可以种植就白色，被占用就红色
        bool canPlant = (_plantMap[row][col] == nullptr);
        _ghostSprite->setColor(canPlant ? Color3B::WHITE : Color3B::RED);
    }
    else {
        // 网格外，就跟随鼠标（或隐藏）
        _ghostSprite->setPosition(mousePos);
        // _ghostSprite->setVisible(false); // 可选：隐藏
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
    }
    else if (_gameState == GameState::PAUSED) {
        resumeGame();
    }
}

void GameScene::pauseGame() {
    if (_gameState == GameState::PAUSED) return;

    _gameState = GameState::PAUSED;
    // 不使用 Director::pause()，避免连 UI 一起停掉，直接靠 _gameState 拦截 update
    AudioManager::getInstance().pauseBackgroundMusic();
    
    showPauseMenu();
}

void GameScene::resumeGame() {
    if (_gameState != GameState::PAUSED) return;

    _gameState = GameState::PLAYING;
    AudioManager::getInstance().resumeBackgroundMusic();

    hidePauseMenu();
}

void GameScene::showPauseMenu() {
    if (_pauseLayer) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 半透明遮罩层
    _pauseLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    _pauseLayer->setContentSize(visibleSize);
    _pauseLayer->setPosition(origin);
    this->addChild(_pauseLayer, 3000);

    // 标题：暂停（为避免编码问题，这里使用英文）
    auto title = Label::createWithTTF("Game Paused", "fonts/Marker Felt.ttf", 48);
    title->setPosition(origin.x + visibleSize.width / 2,
                       origin.y + visibleSize.height * 0.65f);
    title->setColor(Color3B::YELLOW);
    _pauseLayer->addChild(title);

    float marginX = 120.0f;
    float marginY = 60.0f;

    // 左下角：退出（返回主菜单）
    auto exitBtn = ui::Button::create();
    exitBtn->setTitleText("Exit");
    exitBtn->setTitleFontSize(56);
    exitBtn->setTitleFontName("fonts/Marker Felt.ttf");
    exitBtn->setPosition(Vec2(origin.x + marginX,
                              origin.y + marginY));
    exitBtn->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            // 恢复背景音乐，然后回主菜单
    AudioManager::getInstance().resumeBackgroundMusic();
            SceneManager::getInstance().gotoStartScene();
        }
    });
    _pauseLayer->addChild(exitBtn);

    // ���½ǣ�����
    auto resumeBtn = ui::Button::create();
    resumeBtn->setTitleText("Resume");
    resumeBtn->setTitleFontSize(56);
    resumeBtn->setTitleFontName("fonts/Marker Felt.ttf");
    resumeBtn->setPosition(Vec2(origin.x + visibleSize.width - marginX,
                                origin.y + marginY));
    resumeBtn->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            this->resumeGame();
        }
    });
    _pauseLayer->addChild(resumeBtn);
}
    
void GameScene::hidePauseMenu() {
    if (!_pauseLayer) return;

    _pauseLayer->removeFromParent();
    _pauseLayer = nullptr;
}

void GameScene::checkVictoryCondition() {
    // 检查是否所有波次已完成且场上无僵尸
    if (LevelManager::getInstance().isAllWavesCompleted() && _zombies.empty()) {
        endGame(true);
    }
}

void GameScene::checkGameOverCondition() {
    // 检查是否有僵尸到达房子（最左端）
    for (auto zombie : _zombies) {
        if (zombie->getPositionX() < GRID_START_X - 100) { // 到达房子
            endGame(false);
            return;
        }
    }
}

void GameScene::endGame(bool isVictory) {
    if (_gameState != GameState::PLAYING) return; // 防止重复触发
    
    _gameState = isVictory ? GameState::VICTORY : GameState::GAME_OVER;
    
    // 延迟场景转换，让玩家看到最终结果
    this->scheduleOnce([this, isVictory](float dt) {
        if (isVictory) {
            SceneManager::getInstance().gotoVictoryScene();
        }
        else {
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
    
    // 计算背景实际显示尺寸
    Size bgActualSize = Size(bgOriginalSize.width * bgScale, bgOriginalSize.height * bgScale);
    
    // 根据背景实际尺寸调整网格
    // 假设原始设计分辨率为 1024x768，网格占据特定区域
    float bgWidthRatio = bgActualSize.width / 1024.0f;   // 宽度缩放比例
    float bgHeightRatio = bgActualSize.height / 768.0f;  // 高度缩放比例
    
    _actualGridStartX = GRID_START_X * bgWidthRatio + (visibleSize.width - bgActualSize.width) / 2;
    _actualGridStartY = GRID_START_Y * bgHeightRatio + (visibleSize.height - bgActualSize.height) / 2;
    _actualCellWidth = CELL_WIDTH * bgWidthRatio;
    _actualCellHeight = CELL_HEIGHT * bgHeightRatio;
    
    CCLOG("[Info] Grid adjusted - Start: (%.1f, %.1f), Cell: (%.1f, %.1f)", 
          _actualGridStartX, _actualGridStartY, _actualCellWidth, _actualCellHeight);
}

// ��������UI
void GameScene::createShovelUI(cocos2d::Node* uiLayer, float x, float y) {
    // �������Ӳ�
    if (FileUtils::getInstance()->isFileExist("ui/shovelSlot.png")) {
        _shovelSlot = Sprite::create("ui/shovelSlot.png");
        if (_shovelSlot) {
            // ��ȡshovelSlot�ĸ߶ȣ������ƶ�����߶�
            float slotHeight = _shovelSlot->getContentSize().height;
            float adjustedY = y + slotHeight / 2.0f;
            // �����Ӳ۷��ڿ�Ƭ�Ҳ࣬�����ƶ�����߶�
            _shovelSlot->setPosition(x, adjustedY);
            uiLayer->addChild(_shovelSlot);
            CCLOG("[Info] Shovel slot created at (%.1f, %.1f) (adjusted from %.1f by +%.1f)", 
                  x, adjustedY, y, slotHeight / 2.0f);
        }
    } else {
        CCLOG("[Warn] Shovel slot image not found: ui/shovelSlot.png");
    }
    
    // ��������
    if (FileUtils::getInstance()->isFileExist("ui/shovel.png")) {
        _shovel = Sprite::create("ui/shovel.png");
        if (_shovel && _shovelSlot) {
            // ���ӳ�ʼλ���ڲ���
            _shovelOriginalPos = _shovelSlot->getPosition();
            _shovel->setPosition(_shovelOriginalPos);
            uiLayer->addChild(_shovel, 10); // �ϸ߲㼶��ȷ���ڲ�֮��
            CCLOG("[Info] Shovel created at (%.1f, %.1f)", _shovelOriginalPos.x, _shovelOriginalPos.y);
        }
    } else {
        CCLOG("[Warn] Shovel image not found: ui/shovel.png");
    }
}

// ���ò���λ��
void GameScene::resetShovel() {
    if (_shovel) {
        _shovel->setPosition(_shovelOriginalPos);
        _isShovelSelected = false;
        _isShovelDragging = false;
        CCLOG("[Info] Shovel reset to original position");
    }
}

// ��ȡֲ��
void GameScene::tryDigAt(int row, int col) {
    try {
        // �������Χ
        if (row < 0 || row >= _actualGridRows || col < 0 || col >= GRID_COLS) {
            CCLOG("[Warn] Invalid grid position [%d, %d]", row, col);
            return;
        }
        
        // ����λ���Ƿ���ֲ��
        Plant* plant = _plantMap[row][col];
        
        // ����Ƿ���˯���ϵ�ֲ��
        int currentMapId = SceneManager::getInstance().getCurrentMapId();
        bool isWaterRow = (currentMapId == 2 || currentMapId == 4) && (row == 2 || row == 3);
        
        // �����ˮ���У����ȼ���Ƿ�����˯���ϵ�ֲ��
        if (isWaterRow && plant && plant->getName() == "LilyPad") {
            // ���˯�����Ƿ�������ֲ��
            Plant* plantOnLilyPad = nullptr;
            for (auto p : _plants) {
                if (p && !p->isDead() && p != plant) {
                    auto pGridPos = pixelToGrid(p->getPosition());
                    if (pGridPos.first == row && pGridPos.second == col) {
                        plantOnLilyPad = p;
                        break;
                    }
                }
            }
            
            if (plantOnLilyPad) {
                // ��ȡ˯���ϵ�ֲ��
                CCLOG("[Info] Digging plant %s on LilyPad at [%d, %d]", 
                      plantOnLilyPad->getName().c_str(), row, col);
                _plants.eraseObject(plantOnLilyPad);
                plantOnLilyPad->removeFromParent();
                plantOnLilyPad->release();
                // ������Ч
                AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);
                CCLOG("[Info] Successfully dug plant on LilyPad at [%d, %d]", row, col);
                return;
            }
        }
        
        // ���û��˯���ϵ�ֲ���ȡ_plantMap�е�ֲ��
        if (plant == nullptr) {
            CCLOG("[Info] No plant at [%d, %d] to dig", row, col);
            return;
        }
        
        // ��ȡֲ��
        CCLOG("[Info] Digging plant %s at [%d, %d]", plant->getName().c_str(), row, col);
        
        // ��_plantMap���Ƴ�
        _plantMap[row][col] = nullptr;
        
        // ��_plants�б����Ƴ�������
        _plants.eraseObject(plant);
        plant->removeFromParent();
        plant->release();
        
        // ������Ч
        AudioManager::getInstance().playEffect(AudioPath::PLANT_SOUND);
        
        CCLOG("[Info] Successfully dug plant at [%d, %d]", row, col);
        
    } catch (const std::exception& e) {
        CCLOG("[Err] Digging failed: %s", e.what());
    }
}

// �������ֵ������ȴʱ�䣨����5�룬���10�룩
float GameScene::calculateCooldownByCost(int cost) const {
    // �����Χ��Ч������Ĭ��ֵ
    if (_minCost >= _maxCost) {
        return 7.5f; // Ĭ��7.5��
    }
    
    // ���Բ�ֵ���������ֵ��Ӧ5�룬������ֵ��Ӧ10��
    // cooldown = 5 + (cost - minCost) / (maxCost - minCost) * (10 - 5)
    float ratio = static_cast<float>(cost - _minCost) / static_cast<float>(_maxCost - _minCost);
    float cooldown = 5.0f + ratio * 5.0f; // 5�뵽10��֮��
    
    // ȷ���ڷ�Χ��
    if (cooldown < 5.0f) cooldown = 5.0f;
    if (cooldown > 10.0f) cooldown = 10.0f;
    
    return cooldown;
}