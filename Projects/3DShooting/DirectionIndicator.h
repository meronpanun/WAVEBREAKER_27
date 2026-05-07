#pragma once
#include "Vec3.h"
#include <vector>

class Player;
class EnemyBase;

/// <summary>
/// 攻撃された敵の情報構造体
/// </summary>
struct AttackedEnemyInfo
{
    Vec3 position;      // 敵の位置
    float displayTime;  // 表示残り時間
    float maxDisplayTime; // 最大表示時間
    
    AttackedEnemyInfo(const Vec3& pos, float maxTime) :
        position(pos), displayTime(maxTime), maxDisplayTime(maxTime) {}
};

/// <summary>
/// 方向インジケータークラス
/// </summary>
class DirectionIndicator
{
public:
    DirectionIndicator();
    ~DirectionIndicator();

    void Init(Player* player);
    void Update(const std::vector<std::shared_ptr<EnemyBase>>& enemies);
    void Draw();

    /// <summary>
    /// 攻撃された敵の方向を表示する
    /// </summary>
    /// <param name="enemyPos">攻撃した敵の位置</param>
    void ShowAttackedEnemyDirection(const Vec3& enemyPos);

    /// <summary>
    /// 共有リソースの読み込み
    /// </summary>
    static void LoadResources();

    /// <summary>
    /// 共有リソースの解放
    /// </summary>
    static void DeleteResources();

private:
    Player* m_pPlayer;
    std::vector<AttackedEnemyInfo> m_attackedEnemies; // 攻撃された敵の情報
    static int s_indicatorImage; // インジケーターの画像ハンドル
};