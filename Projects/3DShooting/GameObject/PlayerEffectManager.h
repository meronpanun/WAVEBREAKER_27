#pragma once

/// <summary>
/// プレイヤーのエフェクト管理クラス
/// </summary>
class PlayerEffectManager
{
public:
	/// <summary>
	/// エフェクトフィードバック構造体
	/// </summary>
	struct EffectFeedback
	{
		int colorR = 255;
		int colorG = 0;
		int colorB = 0;
		float timer = 0.0f;
		float alpha = 0.0f;
		float duration = 45.0f;
		void Trigger(float d, int r, int g, int b)
		{
			timer	 = d;
			alpha    = 1.0f;
			duration = d;
			colorR   = r; 
			colorG   = g; 
			colorB   = b;
		}
	};

	PlayerEffectManager();
	~PlayerEffectManager() = default;

	/// <summary>
	/// エフェクトの更新
	/// </summary>
	/// <param name="deltaTime">経過時間</param>
	/// <param name="isLowHealth">体力が低いかどうか</param>
	/// <param name="lowHealthBlinkTimer">体力低下の点滅タイマー</param>
	void Update(float deltaTime, bool isLowHealth, float lowHealthBlinkTimer);

	/// <summary>
	/// エフェクトの描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ダメージエフェクトを発動
	/// </summary>
	/// <param name="duration">持続時間</param>
	/// <param name="r">赤成分</param>
	/// <param name="g">緑成分</param>
	/// <param name="b">青成分</param>
	void TriggerDamageEffect(float duration, int r, int g, int b);

	/// <summary>
	/// 回復エフェクトを発動
	/// </summary>
	/// <param name="duration">持続時間</param>
	/// <param name="r">赤成分</param>
	/// <param name="g">緑成分</param>
	/// <param name="b">青成分</param>
	void TriggerHealEffect(float duration, int r, int g, int b);

	/// <summary>
	/// 弾薬エフェクトを発動
	/// </summary>
	/// <param name="duration">持続時間</param>
	/// <param name="r">赤成分</param>
	/// <param name="g">緑成分</param>
	/// <param name="b">青成分</param>
	void TriggerAmmoEffect(float duration, int r, int g, int b);

private:
	/// <summary>
	/// エフェクトフィードバックを描画
	/// </summary>
	/// <param name="effect">エフェクトフィードバック構造体</param>
	void DrawEffectFeedback(EffectFeedback& effect);

	EffectFeedback m_damageEffect;
	EffectFeedback m_healEffect;
	EffectFeedback m_ammoEffect;
};



