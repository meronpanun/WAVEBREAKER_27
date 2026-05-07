#pragma once
#include "CollisionGrid.h"
#include "DxLib.h"
#include "EnemyBase.h"
#include "SpawnAreaInfo.h"
#include "TransformDataLoader.h"
#include "WaveData.h"
#include "SpawnAreaInfo.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class EnemyNormal;
class EnemyRunner;
class EnemyAcid;
class EnemyBoss;
class Bullet;
class Player;
class Effect;

// 敵の出現情報
// ゲーム実行時に生成される「個別の敵の出現予定」
// WaveData の count の数だけ作成される
struct EnemySpawnInfo
{
    std::string enemyType;     // 敵の種類
    VECTOR spawnPos;           // 出現位置
    float spawnTime = 0;       // 出現時間
    bool isSpawned = false;    // 出現済みフラグ
	int spawnLocationType = 0; // スポーン位置タイプ (0:ランダム, 1:下段, 2:中段, 3:上段)
    bool hasShield = false;    // シールドを持っているかどうか
};

/// <summary>
/// ウェーブ管理クラス
/// </summary>
class WaveManager
{
public:
    enum class WaveState
    {
        Interval,   // ウェーブ間の待機中
        Starting,   // 開始演出中
        Active,     // 敵が出現・戦闘中
        Completed   // 全ウェーブ完了
    };

    WaveManager();
    ~WaveManager();

    void Init();
    void Reset();
    void Update();

    /// <summary>
    /// 現在のWave番号を取得
    /// </summary>
    int GetCurrentWave() const { return m_currentWave; }

    /// <summary>
    /// 敵のリストを取得
    /// </summary>
    std::vector<std::shared_ptr<EnemyBase>>& GetEnemyList() { return m_enemyList; }

    /// <summary>
    /// 空間分割グリッドを取得
    /// </summary>
    CollisionGrid& GetCollisionGrid() { return m_collisionGrid; }

    /// <summary>
    /// 敵の死亡時に呼ばれるコールバックを設定
    /// </summary>
    void SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback);

    /// <summary>
    /// 敵ヒット時のコールバックを設定
    /// </summary>
    void SetOnEnemyHitCallback(std::function<void(EnemyBase::HitPart, float)> cb);

    /// <summary>
    /// RoadFloorオブジェクトの範囲を設定
    /// </summary>
    void SetRoadFloorBounds(const VECTOR& minPos, const VECTOR& maxPos);

    /// <summary>
    /// ステージのポリゴンをグリッドに登録する
    /// </summary>
    void RegisterStageToGrid(const std::vector<Stage::StageCollisionData>& collisionData);

    /// <summary>
    /// 敵の一括更新
    /// </summary>
    void UpdateEnemies(std::vector<Bullet>& bullets,
                  const Player::TackleInfo& tackleInfo, const Player& player,
                  const std::vector<Stage::StageCollisionData>& collisionData,
                  Effect* pEffect);

    /// <summary>
    /// 敵の一括描画
    /// </summary>
    void DrawEnemies(const std::vector<Stage::StageCollisionData>& collisionData, bool isTutorial = false);

    // WaveUIの描画はUIManagerで行われます

    /// <summary>
    /// 現在のウェーブ状態を取得
    /// </summary>
    WaveState GetState() const { return m_state; }

    /// <summary>
    /// デバッグUIの描画
    /// </summary>
    void DrawDebugUI();

    /// <summary>
    /// 現在のウェーブがアクティブかどうかを取得
    /// </summary>
    bool IsWaveActive() const { return m_isWaveActive; }

    /// <summary>
    /// すべてのウェーブが完了したかどうかを取得
    /// </summary>
    bool IsAllWavesCompleted() const { return m_haveAllWavesCompleted; }

    /// <summary>
    /// ウェーブ1の敵がロードされたかどうかを取得
    /// </summary>
    bool IsWave1Loaded() const { return m_hasLoadedWave1; }

    /// <summary>
    /// ウェーブ1の敵が実際に出現したかどうかを取得
    /// </summary>
    bool IsWave1EnemySpawned() const { return m_hasSpawnedWave1Enemy; }

    /// <summary>
    /// 現在のスポーンタイマーを取得
    /// </summary>
    float GetSpawnTimer() const { return m_spawnTimer; }

    /// <summary>
    /// ショットチュートリアルがクリアされたかどうかを取得
    /// </summary>
    bool IsShotTutorialCleared() const { return m_hasClearedShotTutorial; }

    /// <summary>
    /// タックルチュートリアルがクリアされたかどうかを取得
    /// </summary>
    bool IsTackleTutorialCleared() const { return m_hasClearedTackleTutorial; }

    /// <summary>
    /// 生存している敵の数を取得
    /// </summary>
    int GetAliveEnemyCount() const;

    /// <summary>
    /// チュートリアル用の敵をスポーンさせる
    /// </summary>
    void SpawnTutorialWave(int tutorialWaveId);

    // デバッグ表示切り替え用 (後方互換性のため維持)
    static void SetDrawSpawnAreas(bool isDraw) { s_shouldDrawSpawnAreas = isDraw; }
    static bool IsDrawSpawnAreas() { return s_shouldDrawSpawnAreas; }

    static void SetShowActiveEnemyCount(bool isShow) { s_shouldShowActiveEnemyCount = isShow; }
    static bool IsShowActiveEnemyCount() { return s_shouldShowActiveEnemyCount; }

    static void SetShowDrawnEnemyCount(bool isShow) { s_shouldShowDrawnEnemyCount = isShow; }
    static bool IsShowDrawnEnemyCount() { return s_shouldShowDrawnEnemyCount; }

    std::shared_ptr<EnemyNormal> GetPooledNormalEnemy();
    std::shared_ptr<EnemyRunner> GetPooledRunnerEnemy();
    std::shared_ptr<EnemyAcid> GetPooledAcidEnemy();
    std::shared_ptr<EnemyBoss> GetPooledBossEnemy();

    // リファクタリング用ヘルパー
    std::shared_ptr<EnemyBase> GetPooledEnemy(const std::string& type);
    void InitEnemyPools();

    // スポーン位置計算ヘルパー
    const SpawnAreaInfo* SelectSpawnArea(int type, const std::string& enemyType, const VECTOR& playerPos, int spawnLocationType);
    VECTOR CalculateRandomSpawnPos(const SpawnAreaInfo& area);

private:
    /// <summary>
    /// ランダムな出現位置を生成
    /// </summary>
    VECTOR GenerateRandomSpawnPos(const VECTOR& playerPos);

    /// <summary>
    /// 出現位置を生成（エリア定義があればそれを使用、なければランダム）
    /// </summary>
    VECTOR GenerateSpawnPos(int type, const std::string& enemyType, const VECTOR& playerPos, int spawnLocationType = 0);

    /// <summary>
    /// 敵を生成
    /// </summary>
    std::shared_ptr<EnemyBase> CreateEnemy(const std::string& enemyType, const VECTOR& spawnPos, bool hasShield = false);

    /// <summary>
    /// 次のウェーブに進む
    /// </summary>
    void NextWave();

    /// <summary>
    /// 現在のウェーブを開始
    /// </summary>
    void StartCurrentWave(const VECTOR& playerPos = VGet(0.0f, 0.0f, 0.0f));

    /// <summary>
    /// 現在のウェーブの敵がすべて倒されたかチェック
    /// </summary>
    bool IsCurrentWaveCleared();

    /// <summary>
    /// 敵が死亡したときの処理
    /// </summary>
    void OnEnemyDeath(const VECTOR& pos);

private:
    // WaveUIはUIManagerで管理されます

    // データリスト
    std::vector<WaveData> m_waveDataList;
    std::vector<EnemySpawnInfo> m_spawnInfoList;
    std::vector<SpawnAreaInfo> m_spawnAreaList;
    std::vector<std::shared_ptr<EnemyBase>> m_enemyList;

    CollisionGrid m_collisionGrid;

    // 敵パラメータ
    std::vector<ObjectTransformData> m_enemyData;

    // 敵のプール
    std::vector<std::shared_ptr<EnemyNormal>> m_enemyNormalPool;
    std::vector<std::shared_ptr<EnemyRunner>> m_enemyRunnerPool;
    std::vector<std::shared_ptr<EnemyAcid>> m_enemyAcidPool;
    std::vector<std::shared_ptr<EnemyBoss>> m_enemyBossPool;

    // コールバック
    std::function<void(const VECTOR&)> m_onEnemyDeathCallback;
    std::function<void(EnemyBase::HitPart, float)> m_onEnemyHitCallback;

    // RoadFloor範囲
    VECTOR m_roadFloorMin;
    VECTOR m_roadFloorMax;

    int m_totalSpawnedCount; // 累計出現数

    // 状態フラグ
    bool m_hasLoadedWave1;
    bool m_hasSpawnedWave1Enemy;
    bool m_hasClearedShotTutorial;
    bool m_hasClearedTackleTutorial;
    bool m_hasSetRoadFloorBounds;
    bool m_isTutorialMode;

    // ウェーブ進行管理
    WaveState m_state;
    int m_currentWave;
    int m_currentSpawnIndex;
    float m_waveTimer;
    float m_spawnTimer;
    float m_waveIntervalTimer;
    bool m_isWaveActive;
    bool m_haveAllWavesCompleted;

    // スタティックメンバ (デバッグフラグ)
    static bool s_shouldDrawSpawnAreas;
    static bool s_shouldShowActiveEnemyCount;
    static bool s_shouldShowDrawnEnemyCount;
};