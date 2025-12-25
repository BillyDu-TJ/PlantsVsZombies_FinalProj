// ʵ�� LevelManager ��
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

    // 加载 assets
    if (doc.HasMember("assets")) {
        const Value& a = doc["assets"];
        _assets.sunBarPath = a["sunBar"].GetString();
        _assets.seedSlotPath = a["seedSlot"].GetString();
    }

    // ֻ���ڱ���·��û�б��ֶ�����ʱ�Ÿ���
    // �������Ա����ͼѡ��ʱ���õı���·��
    if (doc.HasMember("levelInfo")) {
        std::string newBgPath = doc["levelInfo"]["background"].GetString();
        // 如果背景路径没有被手动设置，才从JSON加载
        if (!_isBgPathManuallySet) {
            _assets.bgPath = newBgPath;
        } else {
            // 背景路径已经被手动设置（地图选择），保留它
            CCLOG("[Info] Keeping custom background path: %s (ignoring JSON: %s)", 
                  _assets.bgPath.c_str(), newBgPath.c_str());
        }
    }

    // 获取 waves 数据
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

    // 遍历所有刷新事件
    for (auto& evt : _waves) {
        if (!evt.spawned) {
            allSpawned = false;
            if (_gameTime >= evt.time) {
                // 时间到了，触发刷新
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

bool LevelManager::isAllWavesCompleted() const {
    // 检查是否所有刷新事件都已完成
    for (const auto& wave : _waves) {
        if (!wave.spawned) {
            return false; // ����δ��ɵĲ���
        }
    }
    return true; // ���в��ζ������
}