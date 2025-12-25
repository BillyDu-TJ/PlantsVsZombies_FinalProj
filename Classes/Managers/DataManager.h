// ��ͷ�ļ���������Ϸ���ݹ����� DataManager��������غ��ṩ��Ϸ�и���ʵ������ݡ�
// 2025.11.27 by BillyDu
#ifndef __DATA_MANAGER_H__
#define __DATA_MANAGER_H__

#include <unordered_map>
#include <string>
#include "../Entities/GameDataStructures.h"

class DataManager {
public:
    // C++11 ���ĵ�����ȡ (Meyers' Singleton)
    static DataManager& getInstance();

    // ɾ����������͸�ֵ��������ȷ������Ψһ��
    DataManager(const DataManager&) = delete;
    void operator=(const DataManager&) = delete;

    // ������������ (�׳��쳣)
    void loadData();

    // ��ȡ�ض�ֲ������� (const ���ñ��⿽��)
    const PlantData& getPlantData(int id) const;

    // ��ȡ��ʬ������
	const ZombieData& getZombieData(int id) const;

private:
    DataManager() = default; // ˽�й���

    // ��������ֲ�������ļ��ĸ�������
    void loadPlants(const std::string& filename);

    // �������ݣ�ID -> Data
    std::unordered_map<int, PlantData> _plantDataMap;

	// �������彩ʬ�����ļ��ĸ�������
    void loadZombies(const std::string& filename);

    // ��ʬ���ݻ���
    std::unordered_map<int, ZombieData> _zombieDataMap;
};

#endif // __DATA_MANAGER_H__