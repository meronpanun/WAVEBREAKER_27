#pragma once
#include "AnimationManager.h"
#include "EnemyBase.h"
#include "Effect.h"
#include <memory>
#include "EnemyState.h"

class Bullet;
class Player;
class Collider;
class SphereCollider;
class CapsuleCollider;

/// <summary>
/// 通常の敵クラス
/// </summary>
class EnemyNormal : public EnemyBase
{
public:
    EnemyNormal();
    virtual ~EnemyNormal();

    void Init() override;
    void Update(const EnemyUpdateContext& context) override;
    void Draw() override;

    /// <summary>
    /// デバッグ用の当たり判定を描画する
    /// </summary>
    virtual void DrawCollisionDebug() const override;

    /// <summary>
    /// 死亡時に呼ばれる処理
    /// </summary>
    void OnDeath() override;

    /// <summary>
    /// モデルの読み込み(共有)
    /// </summary>
    static void LoadModel();

    /// <summary>
    /// モデルの解放(共有)
    /// </summary>
    static void DeleteModel();

    static void SetDrawCollision(bool draw) { s_shouldDrawCollision = draw; }
    static bool ShouldDrawCollision() { return s_shouldDrawCollision; }

    static void SetDrawShieldCollision(bool draw) { s_shouldDrawShieldCollision = draw; }
    static bool IsDrawShieldCollision() { return s_shouldDrawShieldCollision; }

private:
    static bool s_shouldDrawCollision;
    static bool s_shouldDrawShieldCollision;

    /// <summary>
    /// どこに当たったかを判定する
    /// </summary>
    /// <param name="rayStart">弾のRayの始点</param>
    /// <param name="rayEnd">弾のRayの終点</param>
    /// <returns>当たった部位</returns>
    HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd,
                         VECTOR& outHtPos, float& outHtDistSq) const override;

    /// <summary>
    /// タックルダメージを受ける処理
    /// </summary>
    void ResetTackleHitFlag() { m_hasTakenTackleDamage = false; }

    /// <summary>
    /// アイテムドロップ時のコールバック関数を設定する
    /// </summary>
    /// <param name="cb">コールバック関数</param>
    void SetOnDropItemCallback(std::function<void(const VECTOR&)> cb);

    /// <summary>
    /// ダメージを受ける処理
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    void TakeDamage(float damage, AttackType type) override;

    /// <summary>
    /// タックルダメージを受ける処理
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    void TakeTackleDamage(float damage) override;

    /// <summary>
    /// ボディコライダーを取得する
    /// </summary>
    /// <returns>ボディコライダー</returns>
    std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;

public:

    /// <summary>
    /// シールドを所持するかどうかを設定し、初期化する
    /// </summary>
    void SetHasShield(bool hasShield);
    void TriggerShieldChainBreak(int delayFrames);

    /// <summary>
    /// シールド状態取得
    /// </summary>
    bool IsShieldBroken() const { return m_isShieldBroken; }
    std::shared_ptr<SphereCollider> GetShieldCollider() const { return m_pShieldCollider; }

protected:
    // ダメージ計算と適用
    void CheckHitAndDamage(std::vector<Bullet>& bullets, Effect* pEffect) override;
    float CalcDamage(float bulletDamage, HitPart part) const override;
    void ApplyBulletDamage(Bullet& bullet, HitPart part, float distSq, Effect* pEffect) override;

private:
    /// <summary>
    /// アニメーションを変更する
    /// </summary>
    /// <param name="newAnimState">新しいアニメーション状態</param>
    /// <param name="loop">ループ再生するかどうか</param>
    void ChangeAnimation(AnimState newAnimState, bool loop);

    /// <summary>
    /// ステートを変更する
    /// </summary>
    /// <param name="newState">新しいステートオブジェクト</param>
    void ChangeState(std::shared_ptr<EnemyState<EnemyNormal>> newState);

    /// <summary>
    /// プレイヤーに攻撃可能かどうかを判定する
    /// </summary>
    /// <param name="player">プレイヤーオブジェクト</param>
    /// <returns>攻撃可能ならtrue</returns>
    bool CanAttackPlayer(const Player& player);
    void BreakShield(const EnemyUpdateContext* context = nullptr);

    void UpdateDeath(const std::vector<Stage::StageCollisionData>& collisionData);

private:
    VECTOR m_headPosOffset; // ヘッドショット判定用オフセット座標

    std::shared_ptr<CapsuleCollider> m_pBodyCollider;      // 体のコライダー
    std::shared_ptr<SphereCollider> m_pHeadCollider;       // 頭のコライダー
    std::shared_ptr<SphereCollider> m_pAttackRangeCollider; // 攻撃範囲のコライダー
    std::shared_ptr<CapsuleCollider> m_pAttackHitCollider;  // 攻撃ヒット判定用のコライダー

    // アイテムドロップ時のコールバック関数
    std::function<void(const VECTOR&)> m_onDropItem;

    AnimState m_currentAnimState; // 現在のアニメーション状態
    std::shared_ptr<EnemyState<EnemyNormal>> m_pCurrentState; // 現在のAIステート

    AnimationManager m_animationManager; // EnemyNormalがアニメーションマネージャーを所有

    int m_attackEndDelayTimer; // 攻撃終了までの遅延タイマー
    int m_damageTimer;         // ダメージ（怯み）タイマー

    float m_animTime;   // アニメーションの経過時間

    bool m_hasTakenTackleDamage;       // 1フレームで複数回ダメージを受けないためのフラグ
    bool m_hasAttackHit;       // 攻撃がヒットしたかどうか
    bool m_isDeadAnimPlaying; // 死亡アニメーション再生中フラグ
    bool m_hasDroppedItem;     // アイテムドロップ済みフラグ

    // 徘徊挙動用
    int m_wanderTimer;     // 徘徊位置更新タイマー
    VECTOR m_wanderOffset; // 徘徊位置オフセット

    // 死亡時吹き飛び用
    bool m_isBlownAway;          // 吹き飛ばされて死亡したか
    VECTOR m_deathKnockbackDir;  // 吹き飛び方向
    float m_deathKnockbackSpeed; // 吹き飛び速度

    // シールド関連メンバ
    bool m_hasShieldConfigured;    // シールドを持っているか
    bool m_isShieldBroken;         // シールドが破壊されたか
    float m_shieldHp;              // シールドHP
    float m_maxShieldHp;           // シールド最大耐久値
    std::shared_ptr<SphereCollider> m_pShieldCollider; // シールドコライダー
    std::vector<int> m_shieldEffectHandles;            // シールドエフェクトハンドル
    float m_shieldRotation;        // シールドの回転角度
    float m_shieldEffectTimer;     // シールドエフェクトの再生タイマー
    bool m_hasPlayedShieldBreakableEffect; // シールド破壊可能エフェクト再生済みフラグ
    int m_shieldChainBreakTimer;   // 連鎖破壊タイマー

    int m_voiceTimer;               // 環境ボイス再生用タイマー
    float m_distToPlayer;           // プレイヤーとの距離
    static int s_modelHandle; // 共有モデルハンドル

    // ステートクラスからのアクセスを許可
    friend class EnemyNormalStateWalk;
    friend class EnemyNormalStateAttack;
    friend class EnemyNormalStateDamage;
    friend class EnemyNormalStateDead;
};