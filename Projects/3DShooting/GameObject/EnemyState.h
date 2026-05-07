#pragma once

struct EnemyUpdateContext;

/// <summary>
/// 敵の状態を管理するステートパターンの基底クラス（テンプレート版）
/// </summary>
template <typename T>
class EnemyState
{
public:
    virtual ~EnemyState() = default;

    /// <summary>
    /// 状態に遷移した時に一度だけ呼ばれる処理
    /// </summary>
    virtual void Enter(T* enemy) {}

    /// <summary>
    /// 毎フレーム呼ばれる更新処理
    /// </summary>
    virtual void Update(T* enemy, const EnemyUpdateContext& context) = 0;

    /// <summary>
    /// 他の状態へ遷移する直前に呼ばれる処理
    /// </summary>
    virtual void Exit(T* enemy) {}
};
