// 该头文件定义了游戏数据管理类 DataManager，负责加载和提供游戏中各种实体的数据。
// 2025.11.27 by BillyDu
#ifndef __DATA_MANAGER_H__
#define __DATA_MANAGER_H__

#include <unordered_map>
#include <string>
#include "../Entities/GameDataStructures.h"

class DataManager {
public:
    // C++11 风格的单例获取 (Meyers' Singleton)
    static DataManager& getInstance();

    // 删除拷贝构造和赋值操作符，确保单例唯一性
    DataManager(const DataManager&) = delete;
    void operator=(const DataManager&) = delete;

    // 加载所有数据 (抛出异常)
    void loadData();

    // 获取特定植物的数据 (const 引用避免拷贝)
    const PlantData& getPlantData(int id) const;

    // 获取僵尸的数据
	const ZombieData& getZombieData(int id) const;

private:
    DataManager() = default; // 私有构造

    // 解析具体植物数据文件的辅助函数
    void loadPlants(const std::string& filename);

    // 缓存数据：ID -> Data
    std::unordered_map<int, PlantData> _plantDataMap;

	// 解析具体僵尸数据文件的辅助函数
    void loadZombies(const std::string& filename);

    // 僵尸数据缓存
    std::unordered_map<int, ZombieData> _zombieDataMap;
};

#endif // __DATA_MANAGER_H__