// 实现 LevelManager 类
// 2025.12.2 by BillyDu
#include "LevelManager.h"
#include "cocos2d.h"
#include "json/document.h"
#include "../Utils/GameException.h"

using namespace rapidjson;

LevelManager& LevelManager::getInstance() {
    static LevelManager instance;
    return instance;
}

void LevelManager::loadLevel(const std::string& filename) {
    _waves.clear();
    _gameTime = 0.0f;
    _isLevelFinished = false;

    std::string fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(filename);
    if (fullPath.empty()) {
        throw GameException("[Err] Level file not found: " + filename);
    }

    std::string content = cocos2d::FileUtils::getInstance()->getStringFromFile(fullPath);
    Document doc;
    doc.Parse(content.c_str());

    if (doc.HasParseError()) throw GameException("[Err] Level JSON parse error");

    // 读取 waves 数组
    if (doc.HasMember("waves") && doc["waves"].IsArray()) {
        const Value& waves = doc["waves"];
        for (SizeType i = 0; i < waves.Size(); i++) {
            const Value& w = waves[i];
            SpawnEvent evt;
            evt.time = w["time"].GetFloat();
            evt.zombieId = w["zombieId"].GetInt();
            evt.row = w["row"].GetInt();
            evt.spawned = false;
            _waves.push_back(evt);
        }
    }
}

void LevelManager::update(float dt, const std::function<void(int, int)>& onSpawnCallback) {
    if (_isLevelFinished) return;

    _gameTime += dt;
    bool allSpawned = true;

    // 遍历所有刷怪事件
    for (auto& evt : _waves) {
        if (!evt.spawned) {
            allSpawned = false;
            if (_gameTime >= evt.time) {
                // 时间到了！触发回调
                if (onSpawnCallback) {
                    onSpawnCallback(evt.zombieId, evt.row);
                }
                evt.spawned = true;
            }
        }
    }

    if (allSpawned) {
        _isLevelFinished = true;
        cocos2d::log("[Info] Level Waves Finished!");
    }
}