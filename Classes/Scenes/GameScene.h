// 游戏主场景头文件，定义了核心架构方法
// 2025.11.27 by BillyDu
#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"

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
};

#endif // __GAME_SCENE_H__