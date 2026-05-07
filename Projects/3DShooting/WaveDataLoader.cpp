#include "WaveDataLoader.h"
#include "DxLib.h"
#include <fstream>
#include <sstream>

std::vector<WaveData> WaveDataLoader::LoadWaveData(const std::string& path)
{
    std::vector<WaveData> waveDataList;
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("Error: Cannot open %s\n", path.c_str());
        return waveDataList;
    }

    std::string line;
    // ヘッダー行をスキップ
    std::getline(file, line);

    // CSVファイルの各行を読み込む
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        WaveData waveData;

        // Wave
        if (!std::getline(ss, token, ',')) continue;
        waveData.wave = std::stoi(token);

        // EnemyType
        if (!std::getline(ss, token, ',')) continue;
        waveData.enemyType = token;

        // Count
        if (!std::getline(ss, token, ',')) continue;
        waveData.count = std::stoi(token);

        // SpawnInterval
        if (!std::getline(ss, token, ',')) continue;
        waveData.spawnInterval = std::stof(token);

        // StartTime
        if (!std::getline(ss, token, ',')) continue;
        waveData.startTime = std::stof(token);

        // WaveInterval
        if (std::getline(ss, token, ','))
        {
            waveData.waveInterval = std::stof(token);
        }
        else
        {
            waveData.waveInterval = 0.0f;
        }

        if (std::getline(ss, token, ','))
        {
            waveData.spawnLocationType = std::stoi(token);
        }
        else
        {
            waveData.spawnLocationType = 0;
        }

        // HasShield (0 または 1)
        if (std::getline(ss, token, ','))
        {
            waveData.hasShield = (std::stoi(token) == 1);
        }
        else
        {
            waveData.hasShield = false;
        }

        waveDataList.push_back(waveData);

        // デバッグ出力
        printf("Loaded: Wave %d, %s, Count %d, Interval %.1f, Start %.1f, Loc %d, Shield %d\n",
               waveData.wave, waveData.enemyType.c_str(), waveData.count,
               waveData.spawnInterval, waveData.startTime,
               waveData.spawnLocationType, waveData.hasShield ? 1 : 0);
    }
    return waveDataList;
}

std::vector<SpawnAreaInfo>
WaveDataLoader::LoadSpawnAreaData(const std::string& path)
{
    std::vector<SpawnAreaInfo> spawnAreaList;
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("Error: Cannot open %s\n", path.c_str());
        return spawnAreaList;
    }

    std::string line;
    // ヘッダー行をスキップ
    std::getline(file, line);

    // CSVファイルの各行を読み込む
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        SpawnAreaInfo info;

        // Type
        if (!std::getline(ss, token, ',')) continue;
        info.type = std::stoi(token);

        // PosX, PosY, PosZ
        float x, y, z;
        if (!std::getline(ss, token, ',')) continue;
        x = std::stof(token);
        if (!std::getline(ss, token, ',')) continue;
        y = std::stof(token);
        if (!std::getline(ss, token, ',')) continue;
        z = std::stof(token);

        // Unity座標系からの変換（100倍）
        info.center = VGet(x * 100.0f, y * 100.0f, z * 100.0f);

        // ScaleX, ScaleY, ScaleZ
        float sx, sy, sz;
        if (!std::getline(ss, token, ',')) continue;
        sx = std::stof(token);
        if (!std::getline(ss, token, ',')) continue;
        sy = std::stof(token);
        if (!std::getline(ss, token, ',')) continue;
        sz = std::stof(token);

        // スケールも100倍する
        info.size = VGet(sx * 100.0f, sy * 100.0f, sz * 100.0f);

        spawnAreaList.push_back(info);

        // デバッグ出力
        printf("Loaded SpawnArea: Type %d, Pos(%.1f, %.1f, %.1f), Size(%.1f, %.1f, %.1f)\n",
               info.type, info.center.x, info.center.y, info.center.z, info.size.x,
               info.size.y, info.size.z);
    }
    return spawnAreaList;
}
