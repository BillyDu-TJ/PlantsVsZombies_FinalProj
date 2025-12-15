// 游戏主场景头文件，定义了核心架构方法
// 2025.11.27 by BillyDu
#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include <vector>
#include <utility>

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

    // --- 核心架构方法 ---

    // 将逻辑坐标 (row, col) 转换为屏幕像素坐标 (x, y)
    // row: 0~4, col: 0~8
    cocos2d::Vec2 gridToPixel(int row, int col);

    // 将屏幕像素坐标转换为逻辑坐标 (row, col)
    // 返回 pair, 如果点击在网格外，返回 {-1, -1}
    std::pair<int, int> pixelToGrid(cocos2d::Vec2 pos);

    // 调试功能：在屏幕上画出格子线
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

	Plant* _plantMap[GRID_ROWS][GRID_COLS]; // 逻辑网格上的植物指针

    // 游戏状态
    int _currentSun = 500; // 初始阳光数
	int _selectedPlantId = -1; // 当前选中的植物ID，-1表示未选择
    GameState _gameState = GameState::PLAYING; // 游戏状态

    // UI Lebel用于显示阳光
	cocos2d::Label* _sunLabel = nullptr;

    // 子弹容器
    cocos2d::Vector<Bullet*> _bullets;

    // 创建子弹的具体逻辑
    void createBullet(cocos2d::Vec2 startPos, int damage);

    // 核心战斗检测 (Collision & AI Check)
    void updateCombatLogic();

    // [UI] 卡片容器
    cocos2d::Vector<SeedCard*> _seedCards;

    // [Ghost] 幽灵精灵（用于预览）
    cocos2d::Sprite* _ghostSprite;

    // [Input] 鼠标移动监听
    void onMouseMove(cocos2d::Event* event);

    // [Helper] 更新幽灵位置
    void updateGhostPosition(cocos2d::Vec2 mousePos);
    
    // [Game Flow] 胜负判定
    void checkVictoryCondition();
    void checkGameOverCondition();
    void endGame(bool isVictory);
    
    // [UI] 暂停按钮
    void createPauseButton();
    void onPauseButtonClicked(cocos2d::Ref* sender);
    void pauseGame();
    void resumeGame();

    // 动态网格参数（如果需要）
    float _actualGridStartX = GRID_START_X;
    float _actualGridStartY = GRID_START_Y;
    float _actualCellWidth = CELL_WIDTH;
    float _actualCellHeight = CELL_HEIGHT;
    
    // 计算实际网格参数的方法
    void calculateGridParameters(cocos2d::Sprite* background);
};

#endif // __GAME_SCENE_H__