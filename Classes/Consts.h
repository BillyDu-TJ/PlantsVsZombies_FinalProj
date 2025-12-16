// 游戏常量定义
// 2025.11.27 by BillyDu
#ifndef __CONSTS_H__
#define __CONSTS_H__

// 网格系统常量
const int GRID_ROWS = 5;    // 草坪行数
const int GRID_COLS = 9;    // 草坪列数

// 网格布局 (像素坐标) - 基于标准分辨率 1280x720
// 这些值是相对于标准背景的基础值，会根据实际背景缩放进行调整
const float GRID_START_X = 100.0f; // 网格左边界
const float GRID_START_Y = 40.0f; // 网格下边界
const float CELL_WIDTH = 100.0f;   // 每格宽度
const float CELL_HEIGHT = 115.0f;  // 每格高度

// 背景相关常量
const float DESIGN_RESOLUTION_WIDTH = 1280.0f;   // 设计分辨率宽度
const float DESIGN_RESOLUTION_HEIGHT = 720.0f;   // 设计分辨率高度

// 游戏状态枚举
enum class GameState {
    MENU,       // 主菜单
    PLAYING,    // 游戏进行中
    PAUSED,     // 暂停
    VICTORY,    // 胜利
    GAME_OVER   // 失败
};

// 音频文件路径
namespace AudioPath {
    const char* const MAIN_MENU_BGM = "audio/mainmenu.mp3";
    const char* const GAME_BGM = "audio/grasswalk.mp3";
    const char* const PLANT_SOUND = "audio/plant.mp3";
    const char* const SHOOT_SOUND = "audio/throw.wav";
    const char* const ZOMBIE_DIE_SOUND = "audio/splat.wav";
    const char* const VICTORY_SOUND = "audio/winmusic.mp3";
    const char* const LOSE_SOUND = "audio/losemusic.wav";
}

#endif // __CONSTS_H__