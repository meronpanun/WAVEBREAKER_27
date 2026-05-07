#include "PlayerEffectManager.h"
#include "EffekseerForDXLib.h"
#include <cmath>

namespace
{
	// ダメージエフェクト
	constexpr float kDamageEffectDuration = 30.0f; // ダメージエフェクトの持続時間
	constexpr int   kDamageEffectColorR   = 255; 
	constexpr int   kDamageEffectColorG   = 0;
	constexpr int   kDamageEffectColorB   = 0;

	// 回復エフェクト
	constexpr float kHealEffectDuration   = 45.0f; // 回復エフェクトの持続時間
	constexpr int   kHealEffectColorR     = 0;
	constexpr int   kHealEffectColorG     = 255;
	constexpr int   kHealEffectColorB     = 0;

	// 弾薬取得エフェクト
	constexpr float kAmmoEffectDuration   = 45.0f; // 弾薬取得エフェクトの持続時間
	constexpr int   kAmmoEffectColorR     = 255;
	constexpr int   kAmmoEffectColorG     = 128;
	constexpr int   kAmmoEffectColorB     = 0;

	// 警告UI関連
	constexpr float kWarningBlinkSpeed	     = 1.5f; // 警告UIの点滅速度
	constexpr float kLowHealthEffectMaxAlpha = 0.7f; // 体力低下UIの最大アルファ値
}

PlayerEffectManager::PlayerEffectManager()
{
}

void PlayerEffectManager::Update(float deltaTime, bool isLowHealth, float lowHealthBlinkTimer)
{
	// ダメージエフェクト
	if (m_damageEffect.timer > 0)
	{
		m_damageEffect.timer -= 1.0f;
		m_damageEffect.alpha -= 1.0f / m_damageEffect.duration;
		if (m_damageEffect.alpha < 0) m_damageEffect.alpha = 0;
	}
	else if (isLowHealth)
	{
		float alpha = (sinf(lowHealthBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
		m_damageEffect.alpha = alpha * kLowHealthEffectMaxAlpha;
		m_damageEffect.colorR = 255;
		m_damageEffect.colorG = 0;
		m_damageEffect.colorB = 0;
	}
	else
	{
		m_damageEffect.alpha = 0.0f;
	}
		
	// 回復エフェクト
	if (m_healEffect.timer > 0)
	{
		m_healEffect.timer -= 1.0f;
		m_healEffect.alpha -= 1.0f / m_healEffect.duration;
		if (m_healEffect.alpha < 0) m_healEffect.alpha = 0;
	}
		
	// 弾薬エフェクト
	if (m_ammoEffect.timer > 0)
	{
		m_ammoEffect.timer -= 1.0f;
		m_ammoEffect.alpha -= 1.0f / m_ammoEffect.duration;
		if (m_ammoEffect.alpha < 0) m_ammoEffect.alpha = 0;
	}
}

void PlayerEffectManager::Draw()
{
	// ダメージエフェクト描画
	DrawEffectFeedback(m_damageEffect);

	// 回復エフェクト描画
	DrawEffectFeedback(m_healEffect);

	// 弾薬エフェクト描画
	DrawEffectFeedback(m_ammoEffect);
}

void PlayerEffectManager::TriggerDamageEffect(float duration, int r, int g, int b)
{
	m_damageEffect.Trigger(duration, r, g, b);
}

void PlayerEffectManager::TriggerHealEffect(float duration, int r, int g, int b)
{
	m_healEffect.Trigger(duration, r, g, b);
}

void PlayerEffectManager::TriggerAmmoEffect(float duration, int r, int g, int b)
{
	m_ammoEffect.Trigger(duration, r, g, b);
}

void PlayerEffectManager::DrawEffectFeedback(EffectFeedback& effect)
{
	if (effect.alpha > 0.0f)
	{
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, nullptr);
		int centerX = screenW * 0.5f;
		int centerY = screenH * 0.5f;
		float maxDistance = sqrtf((float)(screenW * screenW + screenH * screenH)) * 0.5f;
		float edgeWidth = maxDistance * 0.4f;
		const int stepSize = 8;
		for (int y = 0; y < screenH; y += stepSize)
		{
			for (int x = 0; x < screenW; x += stepSize)
			{
				float distanceFromCenter = sqrtf((float)((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY)));
				float distanceFromEdge = maxDistance - distanceFromCenter;
				float edgeIntensity = 0.0f;
				if (distanceFromEdge < edgeWidth)
				{
					edgeIntensity = 1.0f - (distanceFromEdge / edgeWidth);
				}
				int alpha = static_cast<int>(effect.alpha * 180 * edgeIntensity);
				if (alpha > 0)
				{
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
					DrawBox(x, y, x + stepSize, y + stepSize, GetColor(effect.colorR, effect.colorG, effect.colorB), true);
				}
			}
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}



