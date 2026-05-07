#pragma once
#include "SpawnAreaInfo.h"
#include "WaveData.h"
#include <string>
#include <vector>

/// <summary>
/// Waveデータローダー
/// </summary>
class WaveDataLoader
{
public:
    /// <summary>
    /// WaveデータをCSVから読み込む
    /// </summary>
    /// <param name="path">CSVファイルのパス</param>
    /// <returns>読み込んだWaveデータのリスト</returns>
    static std::vector<WaveData> LoadWaveData(const std::string& path);

    /// <summary>
    /// スポーンエリアデータをCSVから読み込む
    /// </summary>
    /// <param name="path">CSVファイルのパス</param>
    /// <returns>読み込んだスポーンエリア情報のリスト</returns>
    static std::vector<SpawnAreaInfo> LoadSpawnAreaData(const std::string& path);
};
