#pragma once
#include "SceneBase.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include "ManagedSound.h"
#include "SafeHandle.h"
#include <vector>
#include <memory>

/// <summary>
/// ゾンビの描画用データ
/// </summary>
struct ZombieData
{
    VECTOR pos;          // 位置
    float angleY;        // Y軸回転
    float animTime;      // 現在のアニメーション再生時間
    float animSpeed;     // アニメーション再生速度
    int animIndex;       // 再生するアニメーションのインデックス（IDLEかATK_HEADなど）
    float totalAnimTime; // アニメーションの総再生時間
};

/// <summary>
/// タイトルシーンクラス
/// </summary>
class SceneTitle : public SceneBase
{
public:
    SceneTitle(bool skipLogo = false);
    virtual ~SceneTitle();

    void Init() override;
    SceneBase* Update() override;
    void Draw() override;

private:
    // リソース管理
    ManagedFont m_font;            // フォント
    ManagedGraph m_titleLogo;      // タイトルロゴ
    ManagedGraph m_banner;         // バナー画像
    int m_zombieVoiceTimer;        // ゾンビボイス再生用タイマー

    // フェード・シーン遷移管理
    int  m_fadeAlpha;       // フェードのアルファ値
    int  m_fadeFrame;       // フェードのフレームカウント
    int  m_sceneFadeAlpha;  // シーンフェードのアルファ値
    bool m_isFadeComplete;  // フェード完了フラグ
    bool m_isFadeOut;       // フェードアウトフラグ
    bool m_isSceneFadeIn;   // シーンフェードインフラグ

    // 演出管理
    int  m_waitFrame;       // 待機フレーム

    // ゲームスタートテキスト演出関連
    int m_gameStartTextAlpha;     // ゲームスタートテキストのアルファ値
    int m_gameStartTextAlphaDir;  // ゲームスタートテキストのアルファ値の増減方向

    // 看板の揺れ演出関連
    float m_billboardShakeOffsetX; // 現在の看板のX軸揺れオフセット
    float m_billboardShakeOffsetY; // 現在の看板のY軸揺れオフセット
    float m_billboardShakePower;   // 看板の揺れの強さ（減衰していく）
    int m_shakeTimer;              // 揺れ計算用タイマー（サイン波用またはランダム更新用）

    // フェンスの揺れ演出関連
    // 画面に映っているフェンス(左:0, 中央:1, 右:2)の3枚のみ揺れパワーを管理する
    float m_fenceShakePower[3]; 

    // 3D背景演出パラメータ
    SafeHandle<ModelDeleter> m_skyDome;    // スカイドームモデル
    SafeHandle<ModelDeleter> m_floorModel; // 床モデル
    SafeHandle<ModelDeleter> m_fenceModel; // フェンスモデル
    SafeHandle<ModelDeleter> m_armoryBillboardModel; // Armory看板モデル
    SafeHandle<ModelDeleter> m_hangarV3Model; // HangarV3モデル
    SafeHandle<ModelDeleter> m_hangarModel; // Hangarモデル
    SafeHandle<ModelDeleter> m_containerModel; // コンテナモデル
    SafeHandle<ModelDeleter> m_zombieModel; // ゾンビモデル
    std::vector<ZombieData> m_zombies;     // 背景に配置するゾンビのリスト

    int m_animIndexIdle;    // IDLEアニメーションのインデックス
    int m_animIndexAtkHead; // ATK_HEADアニメーションのインデックス
};

