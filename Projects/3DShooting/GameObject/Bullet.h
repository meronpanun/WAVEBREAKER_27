#pragma once
#include "AttackType.h"
#include "EffekseerForDXLib.h"
#include "Stage.h"
#include <vector>


/// <summary>
/// 弾クラス
/// </summary>
class Bullet
{
public:
    Bullet(VECTOR position, VECTOR direction, AttackType attackType, float damage = 10.0f, float attenuationStartDist = 0.0f, float attenuationEndDist = 0.0f, float minDamageRatio = 1.0f);
    virtual ~Bullet();

    void Init();
    void Update(const VECTOR &playerPos, const std::vector<Stage::StageCollisionData> &collisionData);
    void Draw() const;

    // 弾の現在の位置を取得
    VECTOR GetPos() const { return m_pos; }
    // 弾の前フレームの位置を取得 (Rayの始点として使用)
    VECTOR GetPrevPos() const { return m_prevPos; }

    /// <summary>
    /// 弾が有効かどうか
    /// </summary>
    /// <returns>有効ならtrue</returns>
    bool IsActive() const { return m_isActive; }

    /// <summary>
    /// 弾の更新
    /// </summary>
    /// <param name="bullets">弾の配列</param>
    /// <param name="playerPos">プレイヤーの位置</param>
    /// <param name="collisionData">ステージのあたり判定データ</param>
    static void UpdateBullets(std::vector<Bullet> &bullets, const VECTOR &playerPos, const std::vector<Stage::StageCollisionData> &collisionData);

    /// <summary>
    /// 弾の描画
    /// </summary>
    /// <param name="bullets">弾の配列</param>
    static void DrawBullets(const std::vector<Bullet> &bullets);

    /// <summary>
    /// 弾を非アクティブ化
    /// </summary>
    void Deactivate();

    /// <summary>
    /// 弾のダメージを取得
    /// </summary>
    /// <returns>ダメージ値</returns>
    float GetDamage() const;

    /// <summary>
    /// 攻撃の種類を取得
    /// </summary>
    /// <returns>攻撃の種類</returns>
    AttackType GetAttackType() const { return m_attackType; }

private:
    VECTOR m_pos;      // 現在の位置
    VECTOR m_prevPos;  // 前フレームの位置 (Rayの始点)
    VECTOR m_spawnPos; // 発射位置
    VECTOR m_dir;      // 進行方向

    float m_speed; // 速度
    float m_damage;

    // 距離減衰パラメータ
    float m_attenuationStartDist; // 減衰開始距離
    float m_attenuationEndDist;   // 減衰終了距離
    float m_minDamageRatio;       // 最低ダメージ倍率

    bool m_isActive;
    AttackType m_attackType; // 攻撃の種類
};