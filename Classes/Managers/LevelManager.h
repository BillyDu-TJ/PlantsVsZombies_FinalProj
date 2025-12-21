// LevelManager 负责管理关卡中的配置和刷新
// 2025.12.2 by BillyDu
#ifndef __LEVEL_MANAGER_H__
#define __LEVEL_MANAGER_H__

#include <vector>
#include <string>
#include <functional> // C++11 std::function

// 定义一个简单的结构存储刷新信息
struct SpawnEvent {
    float time;
    int zombieId;
    int row;
    bool spawned = false; // 是否已刷新
};

// 定义一个结构存储关卡UI信息
struct LevelAssets {
    std::string bgPath;
    std::string sunBarPath;
    std::string seedSlotPath;
};

class LevelManager {
public:
    static LevelManager& getInstance();

    // 加载关卡配置
    void loadLevel(const std::string& filename);

    // 每帧更新，检查是否需要刷新
    // onSpawnCallback: 需要刷新时调用的回调函数 (参数: id, row)
    void update(float dt, const std::function<void(int, int)>& onSpawnCallback);

    // 获取当前关卡资源
    const LevelAssets& getAssets() const { return _assets; }
    
    // 设置背景路径（用于地图选择）
    void setBackgroundPath(const std::string& bgPath) { 
        _assets.bgPath = bgPath; 
        _isBgPathManuallySet = true; // 标记为手动设置
    }

    // 检查是否所有波次都已完成
    bool isAllWavesCompleted() const;

private:
    LevelManager() = default;

    std::vector<SpawnEvent> _waves;
    float _gameTime = 0.0f;
    bool _isLevelFinished = false;
    bool _isBgPathManuallySet = false; // 标记背景路径是否被手动设置
	LevelAssets _assets;
};

#endif // __LEVEL_MANAGER_H__
