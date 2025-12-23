// 游戏场景头文件，定义了游戏的核心架构和功能
// 2025.11.27 by BillyDu
//edited on 2025.12.21 by Zhao
#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include <vector>
#include <utility>
#include <set>

#include "cocos2d.h"
#include "../Entities/Zombie.h"
#include "../Entities/Plant.h"
#include "../Entities/Bullet.h"
#include "../UI/SeedCard.h"
#include "../Consts.h"

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    // 每一帧更新逻辑
    virtual void update(float dt) override;

    // --- 网格架构功能 ---

    // 将逻辑网格坐标 (row, col) 转换为屏幕像素坐标 (x, y)
    // row: 0~4, col: 0~8
    cocos2d::Vec2 gridToPixel(int row, int col);

    // 将屏幕像素坐标转换为逻辑网格坐标 (row, col)
    // 返回 pair，如果转换失败则返回 {-1, -1}
    std::pair<int, int> pixelToGrid(cocos2d::Vec2 pos);

    // 调试功能：在屏幕上绘制网格
    void drawDebugGrid();

    CREATE_FUNC(GameScene);

private:
    cocos2d::Vector<Plant*> _plants;   // 场上的植物列表
    cocos2d::Vector<Zombie*> _zombies; // 场上的僵尸列表

    // 生成僵尸
    void spawnZombie(int id, int row);
    // 种植植物
    void tryPlantAt(int row, int col);
    void selectPlant(int plantId);
    // 挖取植物
    void tryDigAt(int row, int col);

    Plant* _plantMap[6][GRID_COLS]; // 逻辑网格上的植物指针（最大6行，Map2/Map4使用6行，Map1/Map3使用5行）

    // 动态网格行数（根据地图类型：Map1/Map3=5行，Map2/Map4=6行）
    int _actualGridRows = GRID_ROWS;

    // 游戏状态
    int _currentSun = 500; // 初始阳光值
    int _selectedPlantId = -1; // 当前选中的植物ID，-1表示未选中
    GameState _gameState = GameState::PLAYING; // 游戏状态
    
    // 冷却时间计算相关
    int _minCost = 0;  // 最小阳光值
    int _maxCost = 0;  // 最大阳光值
    
    // 根据阳光值计算冷却时间（最少5秒，最多10秒）
    float calculateCooldownByCost(int cost) const;

    // UI Label：用于显示阳光
    cocos2d::Label* _sunLabel = nullptr;

    // 子弹管理
    cocos2d::Vector<Bullet*> _bullets;

    // 创建子弹的通用逻辑
    void createBullet(cocos2d::Vec2 startPos, int damage, BulletType type = BulletType::NORMAL, const std::string& texturePath = "");

    // 创建蘑菇子弹（带动画）
    void createMushroomBullet(cocos2d::Vec2 startPos, int damage);

    // 创建爆炸动画（boom1用于CherryBomb，boom2用于PotatoMine）
    void createExplosionAnimation(cocos2d::Vec2 pos, const std::string& boomType, int damage, int row, int col);

    // 战斗逻辑更新（碰撞检测和AI检查）
    void updateCombatLogic();

    // [UI] 种子卡片
    cocos2d::Vector<SeedCard*> _seedCards;

    // [Ghost] 种植预览（半透明预览）
    cocos2d::Sprite* _ghostSprite;

    // [Shovel] 铲子相关
    cocos2d::Sprite* _shovel = nullptr;        // 铲子精灵
    cocos2d::Sprite* _shovelSlot = nullptr;     // 铲子槽精灵
    cocos2d::Vec2 _shovelOriginalPos;          // 铲子原始位置（在槽中）
    bool _isShovelSelected = false;             // 是否选中了铲子
    bool _isShovelDragging = false;             // 是否正在拖动铲子

    // [Input] 鼠标移动处理
    void onMouseMove(cocos2d::Event* event);

    // [Helper] 更新幽灵位置
    void updateGhostPosition(cocos2d::Vec2 mousePos);

    // [Game Flow] 胜利条件检查
    void checkVictoryCondition();
    void checkGameOverCondition();
    void endGame(bool isVictory);

    // [UI] 暂停按钮 / 暂停菜单
    void createPauseButton();
    void onPauseButtonClicked(cocos2d::Ref* sender);
    void pauseGame();
    void resumeGame();
    void showPauseMenu();
    void hidePauseMenu();

    // [Shovel] 铲子相关函数
    void createShovelUI(cocos2d::Node* uiLayer, float x, float y);
    void resetShovel();

    // 动态网格参数（根据背景大小调整）
    float _actualGridStartX = GRID_START_X;
    float _actualGridStartY = GRID_START_Y;
    float _actualCellWidth = CELL_WIDTH;
    float _actualCellHeight = CELL_HEIGHT;

    // 计算网格实际参数的方法
    void calculateGridParameters(cocos2d::Sprite* background);
    
    // Boss2 ice tracking (grid positions where ice has been placed)
    std::set<std::pair<int, int>> _icePositions;  // (row, col) pairs

    // 暂停菜单层（ESC 或暂停按钮弹出）
    cocos2d::LayerColor* _pauseLayer = nullptr;
};

#endif // __GAME_SCENE_H__