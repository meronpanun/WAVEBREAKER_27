#pragma once
#include "AttackType.h"
#include "Player.h"
#include "Stage.h"
#include <functional>
#include <memory>
#include <vector>

class Bullet;
class Collider;
class SphereCollider;
class CapsuleCollider;
class Effect;
class CollisionGrid;

/// <summary>
/// 敵の更新処理に必要なコンテキスト情報
/// </summary>
struct EnemyUpdateContext
{
    std::vector<Bullet>& bullets;
    const Player::TackleInfo& tackleInfo;
    const Player& player;
    const std::vector<EnemyBase*>& enemyList;
    const std::vector<Stage::StageCollisionData>& collisionData;
    Effect* pEffect;
    const CollisionGrid* collisionGrid;
};

namespace EnemyConstants
{
    constexpr int kDebugDamageDisplayTimer = 120;
}

/// <summary>
/// 敵の基底クラス
/// </summary>
class EnemyBase abstract
{
public:
    EnemyBase();
    virtual ~EnemyBase() = default;

    virtual void Init() abstract;
    virtual void Update(const EnemyUpdateContext& context) abstract;
    virtual void Draw() abstract;

    // デバッグ用カウンタ
    static void ResetDrawCount() { s_drawCount = 0; }
    static void IncrementDrawCount() { s_drawCount++; }
    static int GetDrawCount() { return s_drawCount; }

    static void ResetAIUpdateCount() { s_aiUpdateCount = 0; }
    static void IncrementAIUpdateCount() { s_aiUpdateCount++; }
    static int GetAIUpdateCount() { return s_aiUpdateCount; }

    static void ResetTotalCount() { s_totalCount = 0; }
    static void IncrementTotalCount() { s_totalCount++; }
    static int GetTotalCount() { return s_totalCount; }

    // デバッグ用: ダメージ可視化
    static void SetShowDamage(bool show) { s_shouldShowDamage = show; }
    static bool ShouldShowDamage() { return s_shouldShowDamage; }
    static void DrawDebugDamage(); // ダメージ描画用関数

    /// <summary>
    /// 当たり判定の部位
    /// </summary>
    enum class HitPart
    {
        None,
        Body,
        Head,
        Shield
    };

    /// <summary>
    /// アニメーション状態
    /// </summary>
    enum class AnimState
    {
        Idle,            // 待機
        Walk,            // 歩行
        Back,            // 後退
        Run,             // 走行
        Attack,          // 攻撃
        LongRangeAttack, // 遠距離攻撃
        Damage,          // ダメージ（怯み）
        Dead             // 死亡
    };

    virtual int GetModelHandle() const { return -1; }

    /// <summary>
    /// 弾や攻撃を受ける処理(基底で共通処理)
    /// </summary>
    /// <param name="bullets">弾のリスト</param>
    /// <param name="pEffect">エフェクトクラスのポインタ</param>
    virtual void CheckHitAndDamage(std::vector<Bullet>& bullets, Effect* pEffect = nullptr);

    virtual void TakeDamage(float damage, AttackType type);

    /// <summary>
    /// タックルダメージを受ける処理
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    virtual void TakeTackleDamage(float damage);

    /// <summary>
    /// デバッグ用の当たり判定を描画する
    /// </summary>
    virtual void DrawCollisionDebug() const {}

    /// <summary>
    /// 生存判定
    /// </summary>
    /// <returns>生存しているならtrue</returns>
    virtual bool IsAlive() const { return m_isAlive; }

    /// <summary>
    /// ボスかどうか
    /// </summary>
    virtual bool IsBoss() const { return false; }

    /// <summary>
    /// 位置取得
    /// </summary>
    /// <returns>敵の位置</returns>
    virtual VECTOR GetPos() const { return m_pos; }

    /// <summary>
    /// 位置設定
    /// </summary>
    /// <param name="pos">敵の位置</param>
    virtual void SetPos(const VECTOR& pos) { m_pos = pos; }

    /// <summary>
    /// 体力設定
    /// </summary>
    /// <param name="hp">体力</param>
    virtual void SetHp(float hp) { m_hp = hp; }

    /// <summary>
    /// 最大体力設定
    /// </summary>
    /// <param name="hp">体力</param>
    virtual void SetMaxHp(float hp) { m_maxHp = hp; }

    /// <summary>
    /// 体力取得
    /// </summary>
    float GetHp() const { return m_hp; }

    /// <summary>
    /// 最大体力取得
    /// </summary>
    float GetMaxHp() const { return m_maxHp; }

    /// <summary>
    /// 死亡コールバックを設定する
    /// </summary>
    /// <param name="callback">死亡時に呼ばれるコールバック関数</param>
    virtual void SetOnDeathCallback(std::function<void(const VECTOR&)> callback) { m_onDeathCallback = callback; }

    /// <summary>
    /// 派生クラスでどこに当たったか判定する仮想関数
    /// </summary>
    /// <param name="rayStart">弾のRayの始点</param>
    /// <param name="rayEnd">弾のRayの終点</param>
    /// <returns>当たった部位</returns>
    virtual HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const { return HitPart::None; }

    /// <summary>
    /// タックルでダメージを受けたかどうかのフラグを取得
    /// </summary>
    virtual void ResetTackleHitFlag() abstract;

    /// <summary>
    /// アイテムドロップ時のコールバックを設定する
    /// </summary>
    /// <param name="cb">アイテムドロップ時に呼ばれるコールバック関数</param>
    virtual void SetOnDropItemCallback(std::function<void(const VECTOR&)> cb) {}

    /// <summary>
    /// 敵の状態を設定する
    /// </summary>
    /// <param name="active">アクティブ状態かどうか</param>
    void SetActive(bool active) { m_isActive = active; }

    /// <summary>
    /// 敵がアクティブかどうかを取得する
    /// </summary>
    /// <returns>アクティブならtrue</returns>
    bool IsActive() const { return m_isActive; }

    /// <summary>
    /// ヒット時のコールバックを設定する
    /// </summary>
    /// <param name="cb">ヒット時に呼ばれるコールバック関数</param>
    void SetOnHitCallback(std::function<void(HitPart, float)> cb) { m_onHitCallback = cb; }

    /// <summary>
    /// 最後に当たった攻撃の種類を取得する
    /// </summary>
    /// <returns>最後に当たった攻撃の種類</returns>
    AttackType GetLastAttackType() const { return m_lastAttackType; }

    /// <summary>
    /// 最後に当たった攻撃の種類を設定する
    /// </summary>
    /// <param name="type">最後に当たった攻撃の種類</param>
    void SetLastAttackType(AttackType type) { m_lastAttackType = type; }

    /// <summary>
    /// 最後に当たった部位を取得する
    /// </summary>
    /// <param name="cb">最後に当たった部位</param>
    virtual void SetOnDeathWithTypeCallback(std::function<void(const VECTOR&, AttackType)> cb) { m_onDeathWithTypeCallback = cb; }

    /// <summary>
    /// 敵が死亡した際に呼ばれる処理
    /// </summary>
    virtual void OnDeath() {}

    /// <summary>
    /// ボディコライダーを取得する
    /// </summary>
    /// <returns>ボディコライダー</returns>
    virtual std::shared_ptr<CapsuleCollider> GetBodyCollider() const = 0;

protected:
    /// <summary>
    /// 弾によるダメージを適用する
    /// </summary>
    virtual void ApplyBulletDamage(Bullet& bullet, HitPart part, float distSq, Effect* pEffect);

    /// <summary>
    /// Transformデータをロードする
    /// </summary>
    bool LoadTransformData(const std::string& enemyName);

    /// <summary>
    /// ターゲットに向かって回転する
    /// </summary>
    void RotateTowards(const VECTOR& targetPos, float rotationSpeed);

    /// <summary>
    /// 描画すべきかどうかを判定（カリング）
    /// </summary>
    bool ShouldDraw(float drawDistSq, float nearDistSq, float dotThreshold) const;

    /// <summary>
    /// プレイヤーとの衝突（押し出し）処理
    /// </summary>
    void ResolvePlayerCollision(const std::shared_ptr<CapsuleCollider>& playerCol, float radiusSum, float pushBackEpsilon);

    /// <summary>
    /// 敵同士の衝突（押し出し）処理
    /// </summary>
    void ResolveEnemyCollision(const std::vector<EnemyBase*>& targets, float radius, float pushBackEpsilon);

    /// <summary>
    /// 当たったエフェクトの位置を更新する
    /// </summary>
    void UpdateAttachedEffects();

    struct AttachedEffect {
        int handle;
        VECTOR localOffset;
    };
    std::vector<AttachedEffect> m_attachedEffects;

    /// <summary>
    /// 弾のダメージを計算する
    /// </summary>
    virtual float CalcDamage(float bulletDamage, HitPart part) const;

    /// <summary>
    /// ステージとの当たり判定を更新する
    /// </summary>
    void UpdateStageCollision(const std::vector<Stage::StageCollisionData>& collisionData, const class CollisionGrid* pGrid = nullptr);

    /// <summary>
    /// ターゲットが視界に入っているか（射線が通るか）
    /// </summary>
    static bool IsTargetVisible(const VECTOR& startPos, const VECTOR& targetPos,
                                const std::vector<Stage::StageCollisionData>& stageCollision, const class CollisionGrid* pGrid = nullptr);

    /// <summary>
    /// 放物線軌道の初速度を計算する
    /// </summary>
    static VECTOR CalculateParabolicVelocity(const VECTOR& startPos, const VECTOR& targetPos,
                                             float gravity, float speed);

    /// <summary>
    /// AIの間引き更新処理を行う
    /// </summary>
    void UpdateThrottling(const VECTOR& playerPos);

    /// <summary>
    /// 最も近いヒットした弾を探す
    /// </summary>
    int FindClosestHitBullet(const std::vector<Bullet>& bullets, HitPart& outPart, float& outDistSq) const;

protected:
    VECTOR m_pos;          // 位置
    VECTOR m_targetOffset; // ターゲット座標オフセット

    std::shared_ptr<Player> m_pTargetPlayer;               // ターゲットプレイヤー
    std::function<void(const VECTOR&)> m_onDeathCallback; // 死亡コールバック
    std::function<void(HitPart, float)> m_onHitCallback;   // 部位情報付き
    std::function<void(const VECTOR&, AttackType)> m_onDeathWithTypeCallback;

    HitPart m_lastHitPart; // 最後に当たった部位

    int m_modelHandle;       // モデルハンドル
    int m_hitDisplayTimer;   // ヒット表示タイマー
    int m_attackCooldown;    // 攻撃クールダウンタイマー
    int m_attackCooldownMax; // 攻撃クールダウンの最大値
    int m_attackHitFrame;    // 攻撃ヒットフレーム
    int m_lastTackleId;      // 最後にタックルを受けたID

    float m_hp;          // 体力
    float m_maxHp;       // 最大体力
    float m_attackPower; // 攻撃力
    float m_chaseSpeed;  // 追跡速度

    bool m_isAlive;     // 生存状態フラグ
    bool m_hasTakenTackleDamage; // タックルで既にダメージを受けたか
    bool m_isAttacking; // 攻撃中かどうか
    bool m_isActive;    // デフォルトはアクティブ

    // 重力関連
    float m_verticalVelocity; // 垂直方向の速度
    bool m_isGrounded;        // 接地フラグ

    AttackType m_lastAttackType = AttackType::None;

    // AI Throttling
    int m_updateFrameCount;
    int m_aiUpdateInterval;
    bool m_isSimpleMode;
    bool m_shouldUpdateAI;

protected:
    static int s_drawCount;
    static int s_aiUpdateCount;
    static int s_totalCount;

    // デバッグ用
    static bool s_shouldShowDamage;
    static float s_debugLastDamage;
    static int s_debugDamageTimer;
    static std::string s_debugHitInfo; // ヒット部位情報

public:
    // デバッグ用ゲッター
    static float GetDebugLastDamage() { return s_debugLastDamage; }
    static std::string GetDebugHitInfo() { return s_debugHitInfo; }
    static int GetDebugDamageTimer() { return s_debugDamageTimer; }
};
