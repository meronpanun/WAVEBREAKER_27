#pragma once
#include "UIBase.h"
#include "DxLib.h"
#include "SpawnAreaInfo.h"
#include <string>
#include <vector>

class WaveManager;

/// <summary>
/// WaveUIクラス
/// ウェーブ関連のUI描画およびデバッグ表示を担当
/// </summary>
class WaveUI : public UIBase
{
public:
    WaveUI(WaveManager* waveManager);
    virtual ~WaveUI();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    /// <summary>
    /// スポーンエリアのデバッグ表示
    /// </summary>
    /// <param name="spawnAreaList">スポーンエリアリスト</param>
    /// <param name="isTutorial">チュートリアルかどうか</param>
    void DrawDebugSpawnAreas(const std::vector<SpawnAreaInfo>& spawnAreaList, bool isTutorial);

    /// <summary>
    /// アニメーションの開始
    /// </summary>
    void StartWaveAnimation();

    /// <summary>
    /// アニメーション中かどうか
    /// </summary>
    bool IsAnimating() const { return m_isWaveImageAnimating; }

private:
    WaveManager* m_pWaveManager;

    // ウェーブ画像アニメーション関連
    int m_waveImages[5];                    // 1-5ウェーブ用画像ハンドル
    int m_waveImageAnimTimer;               // ウェーブ画像アニメーションタイマー
    int m_waveImageAnimDuration;            // ウェーブ画像アニメーションの総時間
    int m_waveImageAnimHoldDuration;            // ウェーブ画像アニメーションのホールド時間
    int m_waveImageAnimInitialHoldDuration; // ウェーブ画像アニメーションの初期ホールド時間
    bool m_isWaveImageAnimating;            // ウェーブ画像アニメーション中フラグ
};

