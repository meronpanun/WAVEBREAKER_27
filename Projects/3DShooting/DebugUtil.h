#pragma once
#include "DebugMenu.h"
#include "EffekseerForDXLib.h"  
#include <vector>
#include <string>

/// <summary>
/// デバッグクラス
/// </summary>
class DebugUtil
{
public:
    /// <summary>
	/// 3Dカプセルを描画する
    /// </summary>
	/// <param name="a">端点A</param>
	/// <param name="b">端点B</param>
	/// <param name="radius">半径</param>
	/// <param name="div">分割数</param>
	/// <param name="color">色</param>
	/// <param name="fill">塗りつぶしフラグ</param>
    static void DrawCapsule(const VECTOR& a, const VECTOR& b, float radius, int div, int color, bool fill = false);

    /// <summary>
	/// 3D球を描画する
    /// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="radius">半径</param>
	/// <param name="div">分割数</param>
	/// <param name="color">色</param>
	/// <param name="fill">塗りつぶしフラグ</param>
    static void DrawSphere(const VECTOR& center, float radius, int div, int color, bool fill = false);

    /// <summary>
	/// 2Dメッセージを描画する
    /// </summary>
	/// <param name="x">x座標</param>
	/// <param name="y">y座標</param>
	/// <param name="color">色</param>
	/// <param name="msg">メッセージ</param>
    static void DrawMessage(int x, int y, unsigned int color, const std::string& msg);

    /// <summary>
	/// 2Dフォーマット文字列を描画する
    /// </summary>
	/// <param name="x">x座標</param>
	/// <param name="y">y座標</param>
    /// <param name="color">色</param>
	/// <param name="format">フォーマット文字列</param>
	/// <param name="">可変引数</param>
    static void DrawFormat(int x, int y, unsigned int color, const char* format, ...);

    /// <summary>
	/// ロゴスキップキーが押されたかどうかをチェックする
    /// </summary>
	/// <returns>押されたらtrue</returns>
    static bool IsSkipLogoKeyPressed();

    /// <summary>
	/// デバッグウィンドウの表示/非表示を切り替える
    /// </summary>
    static void ShowDebugWindow();

    /// <summary>
	/// デバッグウィンドウを更新する
    /// </summary>
	/// <returns>デバッグウィンドウが表示されているならtrue</returns>
    static bool IsDebugWindowVisible();

private:
    static bool s_isVisible; // デバッグウィンドウの表示状態
    static DebugMenu s_debugMenu;
};