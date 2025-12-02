// LevelManager 负责管理关卡中的配置加载和刷新
// 2025.12.2 by BillyDu
#ifndef __LEVEL_MANAGER_H__
#define __LEVEL_MANAGER_H__

#include <vector>
#include <string>
#include <functional> // C++11 std::function

// 定义一个简单的结构体存刷怪信息
struct SpawnEvent {
    float time;
    int zombieId;
    int row;
    bool spawned = false; // 标记是否已生成
};

class LevelManager {
public:
    static LevelManager& getInstance();

    // 加载关卡配置
    void loadLevel(const std::string& filename);

    // 每帧更新，检查是否需要刷怪
    // onSpawnCallback: 当需要刷怪时调用的回调函数 (参数: id, row)
    void update(float dt, const std::function<void(int, int)>& onSpawnCallback);

private:
    LevelManager() = default;

    std::vector<SpawnEvent> _waves;
    float _gameTime = 0.0f;
    bool _isLevelFinished = false;
};

#endif // __LEVEL_MANAGER_H__