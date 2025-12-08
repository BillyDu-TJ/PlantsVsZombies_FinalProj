// 该文件定义了植物大战僵尸游戏中逻辑网格的常量参数
// 2025.11.27 by BillyDu
#ifndef __CONSTS_H__
#define __CONSTS_H__

// 逻辑网格定义 (植物大战僵尸标准通常是 5行9列)
const int GRID_ROWS = 5;
const int GRID_COLS = 9;

// 每个格子的像素宽高 (根据你的美术资源调整，暂定 80x100)
const float CELL_WIDTH = 80.0f;
const float CELL_HEIGHT = 100.0f;

// 网格左下角在屏幕上的偏移量 (锚点)
// 假设草地从屏幕坐标 (200, 50) 开始绘制
const float GRID_START_X = 200.0f;
const float GRID_START_Y = 50.0f;

#endif // __CONSTS_H__