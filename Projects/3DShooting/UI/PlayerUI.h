#pragma once
#include "UIBase.h"
#include "PlayerWeaponManager.h"
#include "PlayerShieldSystem.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"


class EnemyBase;
class Player;

/// <summary>
/// プレイヤーのUI描画クラス
/// </summary>
class PlayerUI : public UIBase
{
public:
    PlayerUI(Player* player);
    virtual ~PlayerUI();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Init() override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update(float deltaTime) override;

    /// <summary>
    /// UIの描画
    /// </summary>
    void Draw() override;

private:
    /// <summary>
    /// HPバーの描画
    /// </summary>
    void DrawHPBar(float baseAlpha);

    /// <summary>
    /// 武器UIの描画
    /// </summary>
    void DrawWeaponUI(float baseAlpha);

    /// <summary>
    /// 盾UIの描画
    /// </summary>
    void DrawShieldUI(float baseAlpha);

    /// <summary>
    /// 警告UIの描画
    /// </summary>
    void DrawWarningUI(float baseAlpha);

    /// <summary>
    /// ロックオンUIの描画
    /// </summary>
    void DrawLockOnUI(float baseAlpha);

    /// <summary>
    /// ガード中のテキスト表示
    /// </summary>
    void DrawGuardText(float baseAlpha);
    
    /// <summary>
    /// フォントのリロード（スケール変更時）
    /// </summary>
    /// <param name="scale">UIスケール</param>
    void ReloadFonts(float scale);

    /// <summary>
    /// グラデーション矩形を描画する
    /// </summary>
    void DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor);

private:
    Player* m_pPlayer;

    // UI画像ハンドル
    ManagedGraph m_noAmmoImage;
    ManagedGraph m_noHealthImage;
    ManagedGraph m_arImage;
    ManagedGraph m_noAmmoARImage;
    ManagedGraph m_sgImage;
    ManagedGraph m_noAmmoSGImage;
    ManagedGraph m_healthUiImage;
    ManagedGraph m_shieldImage;
    ManagedGraph m_lockOnUI;

    // フォントハンドル
    ManagedFont m_font;
    ManagedFont m_hpFont;
    ManagedFont m_warningFont;

    // スケール管理
    float m_prevScale;
};

