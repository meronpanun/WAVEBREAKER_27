#pragma once
#include <vector>
#include "DxLib.h"

/// <summary>
/// エフェクトクラス
/// </summary>
class Effect
{
public:
    Effect();
    virtual ~Effect();

    void Init();
    void Update();
    void Draw();

    /// <summary>
    /// マズルフラッシュを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlayMuzzleFlash(float x, float y, float z, float rotX, float rotY, float rotZ);

    int PlayLossOfBlood(float x, float y, float z, float rotX, float rotY, float rotZ);

    int PlayConcentrationLine(float x, float y, float z);

    /// <summary>
    /// ガードエフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <param name="rotX">X軸回転</param>
    /// <param name="rotY">Y軸回転</param>
    /// <param name="rotZ">Z軸回転</param>
    int PlayGuardEffect(float x, float y, float z, float rotX, float rotY, float rotZ);

    /// <summary>
    /// スパークエフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlaySparkEffect(float x, float y, float z, float speed = 1.0f);

    int PlaySparkEffect2(float x, float y, float z);

    /// <summary>
    /// 酸エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlayAcidEffect(float x, float y, float z);

    /// <summary>
    /// 通常弾エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlayNormalBulletEffect(float x, float y, float z);

    /// <summary>
    /// 近接範囲攻撃エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlayCloseRangeAttackEffect(float x, float y, float z);

    /// <summary>
    /// ボスシールドエフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlayBossShieldEffect(float x, float y, float z);

    /// <summary>
    /// シールドヒットエフェクトを再生する
    /// </summary>
    /// <param name="pos">座標</param>
    /// <param name="normal">法線</param>
    int PlayShieldHitEffect(const VECTOR& pos, const VECTOR& normal);

    /// <summary>
    /// シールド破壊可能エフェクトを再生する
    /// </summary>
    /// <param name="pos">座標</param>
    int PlayShieldBreakEffect(const VECTOR& pos);

    /// <summary>
    /// 再生中のエフェクトをすべて停止する
    /// </summary>
    void StopAllEffects();

    /// <summary>
    /// ボスシールドエフェクトの再生時間を取得する
    /// </summary>
    int GetBossShieldEffectDuration() const;

private:
    int m_muzzleFlashEffectHandles[5]; // マズルフラッシュのエフェクトハンドル配列
    int m_lossOfBloodEffectHandle; // 出血エフェクトハンドル
    int m_concentrationLineEffectHandle; // 集中線エフェクトハンドル
    int m_guardEffectHandle; // ガードエフェクトハンドル
    int m_sparkEffectHandle; // スパークエフェクトハンドル
    int m_sparkEffectHandle2; // スパークエフェクトハンドル2
    int m_acidEffectHandle; // 酸エフェクトハンドル
    int m_normalBulletEffectHandle; // 通常弾エフェクトハンドル
    int m_closeRangeAttackEffectHandle; // 近接範囲攻撃エフェクトハンドル
    int m_bossShieldEffectHandle; // ボスシールドエフェクトハンドル
	int m_shieldHitEffectHandle; // シールドヒットエフェクトハンドル
    int m_shieldBreakEffectHandle; // シールド破壊可能エフェクトハンドル

    std::vector<int> m_playingEffectHandles; // 再生中のエフェクトハンドル
};
