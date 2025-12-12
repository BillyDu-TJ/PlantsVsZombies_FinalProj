// 该文件定义了植物大战僵尸游戏中逻辑网格的常量参数
// 2025.11.27 by BillyDu
#ifndef __CONSTS_H__
#define __CONSTS_H__

const float SCALE_RATIO = 1.2;

// 逻辑网格定义 (植物大战僵尸标准通常是 5行9列)
const int GRID_ROWS = 5;
const int GRID_COLS = 9;

// 每个格子的像素宽高
const float CELL_WIDTH = 80.0f * SCALE_RATIO;
const float CELL_HEIGHT = 100.0f * SCALE_RATIO;

// 网格左下角在屏幕上的偏移量 (锚点)
const float GRID_START_X = 250.0f * SCALE_RATIO;
const float GRID_START_Y = 30.0f * SCALE_RATIO;

#endif // __CONSTS_H__