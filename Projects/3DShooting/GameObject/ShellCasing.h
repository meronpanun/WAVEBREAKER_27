#pragma once
#include "DxLib.h"
#include <vector>

/// <summary>
/// 弾丸の薬莢クラス
/// </summary>
class ShellCasing
{
public:
    ShellCasing(const VECTOR& pos, const VECTOR& dir);

    void Update();
    void Draw() const;

    /// <summary>
	/// 薬莢の更新
    /// </summary>
	/// <param name="shellCasings">薬莢コンテナ</param>
    static void UpdateShellCasings(std::vector<ShellCasing>& shellCasings);

    /// <summary>
	/// 薬莢の描画
    /// </summary>
	/// <param name="shellCasings">薬莢コンテナ</param>
    static void DrawShellCasings(const std::vector<ShellCasing>& shellCasings);

public:
    /// <summary>
    /// モデル・SEの読み込み
    /// </summary>
    static void LoadResources();

    /// <summary>
	/// モデル・SEの解放
    /// </summary>
    static void DeleteResources();

private:
    VECTOR m_pos;
    VECTOR m_velocity;
    VECTOR m_rotation;

    int m_modelHandle;
    int m_lifeTime;

    static int s_modelHandle;
};
