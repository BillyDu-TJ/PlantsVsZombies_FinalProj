// 游戏主场景头文件，定义了核心架构方法
// 2025.11.27 by BillyDu
#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include <vector>
#include <utility>

#include "cocos2d.h"
#include "../Entities/Zombie.h"
#include "../Entities/Plant.h"
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

    // UI Lebel用于显示阳光
	cocos2d::Label* _sunLabel = nullptr;
};

#endif // __GAME_SCENE_H__