// 实现 DataManager 类，负责加载和管理游戏数据
#include "DataManager.h"
#include "cocos2d.h"
#include "json/document.h" // RapidJSON
#include "../Utils/GameException.h"

using namespace rapidjson;

DataManager& DataManager::getInstance() {
    static DataManager instance;
    return instance;
}

void DataManager::loadData() {
    // 集中加载所有数据文件
    try {
        loadPlants("data/plants.json");
        loadZombies("data/zombies.json"); // 待扩展
        cocos2d::log("[Info] All data loaded successfully.");
    }
    catch (const std::exception& e) {
        // 捕获异常并记录，然后再次抛出给上层处理
        cocos2d::log("[Err] CRITICAL ERROR loading data: %s", e.what());
        throw; // 重新抛出，因为数据加载失败游戏无法运行
    }
}

void DataManager::loadPlants(const std::string& filename) {
    std::string fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(filename);

    if (fullPath.empty()) {
        throw GameException("[Err] Config file not found: " + filename);
    }

    std::string content = cocos2d::FileUtils::getInstance()->getStringFromFile(fullPath);

    Document doc;
    doc.Parse(content.c_str());

    if (doc.HasParseError()) {
        throw GameException("[Err] JSON Parse error in " + filename);
    }

    if (!doc.IsObject()) {
        throw GameException("[Err] Invalid JSON format in " + filename);
    }

    // 遍历 JSON 数据
    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
        // key 是字符串 ID ("1001")，需要转为 int
        int id = std::stoi(it->name.GetString());
        const auto& val = it->value;

        PlantData data;

        // 使用访问器检查字段是否存在
        if (!val.HasMember("name")) throw GameException("[Err] Missing 'name' in plant " + std::to_string(id));

        data.name = val["name"].GetString();
        data.type = val.HasMember("type") ? val["type"].GetString() : "unknown";
        data.hp = val["hp"].GetInt();
        data.cost = val["cost"].GetInt();
        data.cooldown = val["cooldown"].GetFloat();
        data.attack = val.HasMember("attack") ? val["attack"].GetInt() : 0;
        data.texturePath = val["texture"].GetString();

        if (val.HasMember("attackSpeed")) {
            data.attackSpeed = val["attackSpeed"].GetFloat();
        }
        else {
            data.attackSpeed = 0.0f;
            CCLOG("[Warn] Plant %d missing 'attackSpeed'", id);
        }

        // 加载动画配置
        if (val.HasMember("animations") && val["animations"].IsObject()) {
            const auto& animsObj = val["animations"];
            for (auto animIt = animsObj.MemberBegin(); animIt != animsObj.MemberEnd(); ++animIt) {
                std::string animName = animIt->name.GetString();
                const auto& animVal = animIt->value;
                
                AnimationConfig animConfig;
                animConfig.frameFormat = animVal["frameFormat"].GetString();
                animConfig.frameCount = animVal["frameCount"].GetInt();
                animConfig.frameDelay = animVal.HasMember("frameDelay") ? 
                    animVal["frameDelay"].GetFloat() : 0.1f;
                animConfig.loopCount = animVal.HasMember("loopCount") ? 
                    animVal["loopCount"].GetInt() : -1;
                
                if (animVal.HasMember("defaultTexture")) {
                    animConfig.defaultTexture = animVal["defaultTexture"].GetString();
                } else {
                    // 如果没有指定，使用第一帧
                    char defaultPath[256];
                    snprintf(defaultPath, sizeof(defaultPath), animConfig.frameFormat.c_str(), 1);
                    animConfig.defaultTexture = defaultPath;
                }
                
                if (animVal.HasMember("onComplete")) {
                    animConfig.onComplete = animVal["onComplete"].GetString();
                }
                
                data.animations[animName] = animConfig;
            }
        }
        
        // 加载默认动画名称
        if (val.HasMember("defaultAnimation")) {
            data.defaultAnimation = val["defaultAnimation"].GetString();
        }
        
        // 加载卡片图片路径（可选）
        if (val.HasMember("cardImage")) {
            data.cardImage = val["cardImage"].GetString();
        }

        // 插入到 Map 中
        _plantDataMap[id] = data;
    }
}

const PlantData& DataManager::getPlantData(int id) const {
    auto it = _plantDataMap.find(id);
    if (it == _plantDataMap.end()) {
        throw GameException("[Err] Plant ID not found: " + std::to_string(id));
    }
    return it->second;
}

// [待实现] 实现 getZombieData
const ZombieData& DataManager::getZombieData(int id) const {
    auto it = _zombieDataMap.find(id);
    if (it == _zombieDataMap.end()) {
        throw GameException("Zombie ID not found: " + std::to_string(id));
    }
    return it->second;
}

// [待实现] 实现 loadZombies (逻辑类似 loadPlants)
void DataManager::loadZombies(const std::string& filename) {
    std::string fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(filename);
    if (fullPath.empty()) throw GameException("Config file not found: " + filename);

    std::string content = cocos2d::FileUtils::getInstance()->getStringFromFile(fullPath);
    Document doc;
    doc.Parse(content.c_str());

    if (doc.HasParseError() || !doc.IsObject()) {
        throw GameException("JSON Parse error in " + filename);
    }

    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
        int id = std::stoi(it->name.GetString());
        const auto& val = it->value;

        ZombieData data;
        // 安全地读取
        data.name = val.HasMember("name") ? val["name"].GetString() : "Unknown";
        data.hp = val.HasMember("hp") ? val["hp"].GetInt() : 100;
        data.damage = val.HasMember("damage") ? val["damage"].GetInt() : 10;
        data.speed = val.HasMember("speed") ? val["speed"].GetFloat() : 10.0f;
        data.attackInterval = val.HasMember("attackInterval") ? val["attackInterval"].GetFloat() : 1.0f;
        data.texturePath = val.HasMember("texture") ? val["texture"].GetString() : "";

        // 加载动画配置
        if (val.HasMember("animations") && val["animations"].IsObject()) {
            const auto& animsObj = val["animations"];
            for (auto animIt = animsObj.MemberBegin(); animIt != animsObj.MemberEnd(); ++animIt) {
                std::string animName = animIt->name.GetString();
                const auto& animVal = animIt->value;
                
                AnimationConfig animConfig;
                animConfig.frameFormat = animVal["frameFormat"].GetString();
                animConfig.frameCount = animVal["frameCount"].GetInt();
                animConfig.frameDelay = animVal.HasMember("frameDelay") ? 
                    animVal["frameDelay"].GetFloat() : 0.1f;
                animConfig.loopCount = animVal.HasMember("loopCount") ? 
                    animVal["loopCount"].GetInt() : -1;
                
                if (animVal.HasMember("defaultTexture")) {
                    animConfig.defaultTexture = animVal["defaultTexture"].GetString();
                } else {
                    // 如果没有指定，使用第一帧
                    char defaultPath[256];
                    snprintf(defaultPath, sizeof(defaultPath), animConfig.frameFormat.c_str(), 1);
                    animConfig.defaultTexture = defaultPath;
                }
                
                if (animVal.HasMember("onComplete")) {
                    animConfig.onComplete = animVal["onComplete"].GetString();
                }
                
                data.animations[animName] = animConfig;
            }
        }
        
        // 加载默认动画名称
        if (val.HasMember("defaultAnimation")) {
            data.defaultAnimation = val["defaultAnimation"].GetString();
        }

        _zombieDataMap[id] = data;
        CCLOG("[Info] Loaded Zombie ID: %d (%s) with %zu animations", id, data.name.c_str(), data.animations.size());
    }
}
