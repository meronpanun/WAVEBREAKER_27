#pragma once
#include "AnimationManager.h"
#include "EnemyBase.h"
#include "DxLib.h"
#include <memory>
#include "EnemyState.h"

class Bullet;
class Player;
class Collider;
class CapsuleCollider;
class SphereCollider;

/// <summary>
/// 走る敵クラス
/// </summary>
class EnemyRunner : public EnemyBase
{
public:
    EnemyRunner();
    virtual ~EnemyRunner();

    void Init() override;
    void Update(const EnemyUpdateContext& context) override;
    void Draw() override;

    void OnDeath() override;

    /// <summary>
    /// デバッグ用の当たり判定を描画する
    /// </summary>
    virtual void DrawCollisionDebug() const override;

    static void LoadModel();
    static void DeleteModel();

    static void SetDrawCollision(bool draw) { s_shouldDrawCollision = draw; }
    static bool ShouldDrawCollision() { return s_shouldDrawCollision; }

private:
    static bool s_shouldDrawCollision;

    /// <summary>
    /// どこに当たったかを判定する
    /// </summary>
    /// <param name="rayStart">弾のRayの始点</param>
    /// <param name="rayEnd">弾のRayの終点</param>
    /// <returns>当たった部位</returns>
    HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const override;

    /// <summary>
    /// タックルダメージを受ける処理
    /// </summary>
    void ResetTackleHitFlag() { m_hasTakenTackleDamage = false; }

    /// <summary>
    /// アイテムドロップ時のコールバック関数を設定する
    /// </summary>
    /// <param name="cb">コールバック関数</param>
    void SetOnDropItemCallback(std::function<void(const VECTOR&)> cb);

    // ダメージ処理
    void TakeDamage(float damage, AttackType type) override;
    void TakeTackleDamage(float damage) override;

    /// <summary>
    /// ボディコライダーを取得する
    /// </summary>
    /// <returns>ボディコライダー</returns>
    std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;

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
    void ChangeState(std::shared_ptr<EnemyState<EnemyRunner>> newState);

    /// <summary>
    /// プレイヤーに攻撃可能かどうかを判定
    /// </summary>
    /// <param name="player">プレイヤーオブジェクト</param>
    /// <param name="checkRadius">判定半径(-1の場合はデフォルト)</param>
    /// <returns>攻撃可能ならtrue</returns>
    bool CanAttackPlayer(const Player& player, float checkRadius = -1.0f);

    void UpdateDeath(const std::vector<Stage::StageCollisionData>& collisionData);

private:
    VECTOR m_headPosOffset; // ヘッドショット判定用座標

    std::shared_ptr<CapsuleCollider> m_pBodyCollider;      // 体のコライダー
    std::shared_ptr<SphereCollider> m_pHeadCollider;       // 頭のコライダー
    std::shared_ptr<SphereCollider> m_pAttackRangeCollider; // 攻撃範囲のコライダー
    std::shared_ptr<CapsuleCollider> m_pAttackHitCollider;  // 攻撃ヒット判定用のコライダー

    // アイテムドロップ時のコールバック関数
    std::function<void(const VECTOR&)> m_onDropItem;

    AnimState m_currentAnimState;        // 現在のアニメーション状態
    std::shared_ptr<EnemyState<EnemyRunner>> m_pCurrentState; // 現在のAIステート

    AnimationManager m_animationManager; // EnemyRunnerがアニメーションマネージャーを所有

    int m_attackEndDelayTimer; // 攻撃終了までの遅延タイマー

    float m_animTime;   // アニメーションの経過時間

    bool m_hasTakenTackleDamage;       // 1フレームで複数回ダメージを受けないためのフラグ
    bool m_hasAttackHit;       // 攻撃がヒットしたかどうか
    bool m_isDeadAnimPlaying; // 死亡アニメーション再生中フラグ
    bool m_hasDroppedItem;     // アイテムドロップ済みフラグ

    // 徘徊挙動用
    int m_wanderTimer;     // 徘徊位置更新タイマー
    VECTOR m_wanderOffset; // 徘徊位置オフセット

    // 回避挙動用
    float m_evadeSwitchTimer; // 回避方向切り替えタイマー
    bool m_isEvadingRight;    // 現在右に避けているか


    float m_distToPlayer;         // プレイヤーとの距離
    float m_damageSECooldown;    // ダメージSE再生用クールタイム
    static int s_modelHandle; // 共有モデルハンドル

    // ステートクラスからのアクセスを許可
    friend class EnemyRunnerStateRun;
    friend class EnemyRunnerStateAttack;
    friend class EnemyRunnerStateDead;
};