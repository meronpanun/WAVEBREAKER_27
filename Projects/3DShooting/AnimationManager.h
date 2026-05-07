#pragma once
#include "EffekseerForDXLib.h"
#include "EnemyBase.h"
#include <string>
#include <map>

/// <summary>
/// アニメーション管理クラス
/// </summary>
class AnimationManager
{
public:
    AnimationManager();
    ~AnimationManager();

    void Update(int modelHandle, float delta);

    /// <summary>
    /// モデルにアニメーションをアタッチし、再生を開始する
    /// </summary>
    /// <param name="modelHandle">アニメーションを適用するモデルのハンドル</param>
    /// <param name="animName">再生するアニメーションの名前</param>
    /// <param name="loop">ループ再生するかどうか</param>
    /// <returns>アニメーションの総時間。アニメーションが見つからない場合は0</returns>
    float PlayAnimation(int modelHandle, const std::string& animName, bool loop);

    /// <summary>
    /// アニメーションの時間を更新する
    /// </summary>
    /// <param name="modelHandle">アニメーションを更新するモデルのハンドル</param>
    /// <param name="animTime">現在のアニメーション再生時間</param>
    void UpdateAnimationTime(int modelHandle, float animTime);

    /// <summary>
    /// 指定したアニメーションの総時間を取得する
    /// </summary>
    /// <param name="modelHandle">アニメーションを取得するモデルのハンドル</param>
    /// <param name="animName">アニメーションの名前</param>
    /// <returns>アニメーションの総時間。見つからない場合は0</returns>
    float GetAnimationTotalTime(int modelHandle, const std::string& animName) const;

    /// <summary>
    /// 現在アタッチされているアニメーションのハンドルを取得
    /// </summary>
    /// <param name="modelHandle">アニメーションを取得するモデルのハンドル</param>
    /// <returns>現在アタッチされているアニメーションのハンドル。見つからない場合は-1</returns>
    int GetCurrentAttachedAnimHandle(int modelHandle) const;

    /// <summary>
    /// 現在アタッチされているアニメーションのハンドルをリセットする
    /// </summary>
    /// <param name="modelHandle">アニメーションをリセットするモデルのハンドル</param>
    void ResetAttachedAnimHandle(int modelHandle);

    /// <summary>
    /// 指定した状態に対応するアニメーション名を設定する
    /// </summary>
    /// <param name="state">アニメーションの状態</param>
    /// <param name="animName">アニメーションの名前</param>
    void SetAnimName(EnemyBase::AnimState state, const std::string& animName);

    /// <summary>
    /// 指定したモデルのアニメーションを再生する
    /// </summary>
    /// <param name="modelHandle">モデルのハンドル</param>
    /// <param name="state">アニメーションの状態</param>
    /// <param name="loop">ループ再生するかどうか</param>
    /// <returns>>アニメーションの総時間。アニメーションが見つからない場合は0</returns>
    float PlayState(int modelHandle, EnemyBase::AnimState state, bool loop);

    /// <summary>
    /// 指定したモデルのアニメーションが終了しているかどうかを判定する
    /// </summary>
    /// <param name="modelHandle">モデルのハンドル</param>
    /// <returns>trueならアニメーションが終了している</returns>
    bool IsAnimationFinished(int modelHandle) const;

private:
    /// <summary>
    /// モデルハンドルとアニメーション名からDxLibのアニメーションインデックスを取得
    /// </summary>
    /// <param name="modelHandle">モデルのハンドル</param>
    /// <param name="animName">アニメーションの名前</param>
    /// <returns>DxLibのアニメーションインデックス。見つからない場合は-1</returns>
    int GetAnimIndexInternal(int modelHandle, const std::string& animName);

private:
    // モデルハンドルとアニメーション名ごとのDxLibアニメーションインデックスをキャッシュ
    std::map<int, std::map<std::string, int>> m_animIndexesCache;
    // モデルハンドルと現在アタッチされているDxLibアニメーションハンドル
    std::map<int, int> m_attachedAnimHandles;
    // モデルハンドルと現在アタッチされているアニメーションの総時間
    std::map<int, float> m_currentAnimTotalTimes;

    // 状態→アニメーション名マッピング
    std::map<EnemyBase::AnimState, std::string> m_animStateToAnimName;
    // モデルごとの現在のAnimState
    std::map<int, EnemyBase::AnimState> m_modelCurrentState;
    // モデルごとの現在のアニメーション経過時間
    std::map<int, float> m_modelAnimTime;
};