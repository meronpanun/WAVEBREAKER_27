#pragma once
#include "UIBase.h"
#include "DxLib.h"
#include "ManagedFont.h"
#include <vector>
#include <memory>

class EnemyBase;
class WaveManager;

/// <summary>
/// ボスUI描画クラス
/// </summary>
class BossUI : public UIBase
{
public:
	BossUI(WaveManager* waveManager);
	virtual ~BossUI();

	void Init() override;
	void Update(float deltaTime) override;
	void Draw() override;

private:
	/// <summary>
	/// ボスHPバーの描画
	/// </summary>
	/// <param name="hp">現在の体力</param>
	/// <param name="maxHp">最大体力</param>
	void DrawBossHPBar(float hp, float maxHp);

	/// <summary>
	/// フォントのリロード（スケール変更時に呼び出す）
	/// </summary>
	/// <param name="scale">新しいスケール</param>
	void ReloadFonts(float scale);

	/// <summary>
	/// グラデーションボックスの描画（上部と下部で色が変わる）
	/// </summary>
	void DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor);

private:
	WaveManager* m_pWaveManager;
	float m_healthBarAnim; // HPバーアニメーション用体力値
	ManagedFont m_font;    // フォントハンドル

	// スケール管理
	float m_prevScale;

};

