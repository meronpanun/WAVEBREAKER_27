#include "BossUI.h"
#include "EnemyBase.h"
#include "WaveManager.h"
#include "Game.h"
#include <algorithm>

namespace 
{
    // ... (定数はそのまま)
    // ボスHPバー関連
    constexpr int kBossHpBarWidth  = 900;
    constexpr int kBossHpBarHeight = 24;
    constexpr int kBossHpBarY      = 225;
    constexpr int kBossHpTextY     = 170;

    // 色関連
    constexpr unsigned int kColorWhite       = 0xffffff;
    constexpr unsigned int kColorHpBarBg     = 0x303030;
    constexpr unsigned int kColorHpBarTop    = 0xff4040; // ボスHPバー上部（明るい赤）
    constexpr unsigned int kColorHpBarBottom = 0x800000; // ボスHPバー下部（暗い赤）
    constexpr unsigned int kColorHpBarFill   = 0xcc0000; // ボスは真紅（バックアップ用）
    constexpr unsigned int kColorHpBarDamage = 0xaaaaaa; // 被ダメ時はグレー
    constexpr unsigned int kColorHpBarBorder = 0xffffff;
    constexpr float kAnimSpeed = 0.05f; // HPバーの追従速度

    constexpr int kShadowOffset = 2; // 文字の影オフセット
} 

BossUI::BossUI(WaveManager* waveManager)
    : m_pWaveManager(waveManager)
    , m_healthBarAnim(0.0f)
    , m_font("ＭＳ ゴシック", 36, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_prevScale(1.0f)
{
}

BossUI::~BossUI() 
{
}

void BossUI::Init()
{
    ReloadFonts(1.0f);
}

void BossUI::Update(float deltaTime)
{
    // スケール変更検知
    float currentScale = Game::GetUIScale();
    if (fabsf(currentScale - m_prevScale) > 0.001f) 
    {
        ReloadFonts(currentScale);
        m_prevScale = currentScale;
    }
}

void BossUI::Draw() 
{
    if (!m_pWaveManager) return;

    // 生存しているボスを探す
    const auto& enemyList = m_pWaveManager->GetEnemyList();
    EnemyBase *pBoss = nullptr;
    for (auto &enemy : enemyList) 
    {
        if (enemy->IsBoss() && enemy->IsAlive()) 
        {
            pBoss = enemy.get();
            break;
        }
    }

    if (!pBoss) 
    {
        m_healthBarAnim = 0.0f;
        return;
    }

    float hp = pBoss->GetHp();
    float maxHp = pBoss->GetMaxHp();

    // 初回時やリセット用
    if (m_healthBarAnim <= 0.0f) 
    {
        m_healthBarAnim = hp;
    }

    // アニメーション更新
    // タイムスケールを考慮
    float speed = kAnimSpeed * Game::GetTimeScale();
    if (m_healthBarAnim > hp) 
    {
        m_healthBarAnim -= (m_healthBarAnim - hp) * speed;
        if (m_healthBarAnim < hp) m_healthBarAnim = hp;
    } 
    else if (m_healthBarAnim < hp) 
    {
        m_healthBarAnim = hp;
    }

    DrawBossHPBar(hp, maxHp);
}


void BossUI::DrawBossHPBar(float hp, float maxHp) 
{
    // スケール変更検知
    float currentScale = Game::GetUIScale();
    if (fabsf(currentScale - m_prevScale) > 0.001f) 
    {
        ReloadFonts(currentScale);
        m_prevScale = currentScale;
    }

    int screenW = Game::GetScreenWidth();
     float scale = Game::GetUIScale();

    int barW = static_cast<int>(kBossHpBarWidth * scale);
    int barH = static_cast<int>(kBossHpBarHeight * scale);
    int barY = static_cast<int>(kBossHpBarY * scale);

    int barX = (screenW - barW) * 0.5f;

    // HP割合
    float hpRate = hp / maxHp;
    float animRate = m_healthBarAnim / maxHp;

    // 背景
    DrawBox(barX, barY, barX + barW, barY + barH, kColorHpBarBg, true);

    // アニメーションバー（ダメージ演出用）
    if (m_healthBarAnim > hp) 
    {
        int animW = static_cast<int>(barW * animRate);
        DrawBox(barX, barY, barX + animW, barY + barH, kColorHpBarDamage, true);
    }

    // HPバー本体（グラデーション）
    if (hpRate > 0.0f)
    {
        int hpW = static_cast<int>(barW * hpRate);
        DrawGradientBox(barX, barY, barX + hpW, barY + barH, kColorHpBarTop, kColorHpBarBottom);
        
        // 光沢ライン（上部20%）
        int glossH = static_cast<int>(barH * 0.2f);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(barX, barY, barX + hpW, barY + glossH, 0xffffff, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // 枠
    DrawBox(barX, barY, barX + barW, barY + barH, kColorHpBarBorder, false);

    // ボス名テキスト（影付き）
    const char *bossName = "BOSS";
    int textW = GetDrawStringWidthToHandle(bossName, static_cast<int>(strlen(bossName)), m_font);
    int textX = static_cast<int>((screenW - textW) * 0.5f);

    int textY = static_cast<int>(kBossHpTextY * scale);
    
    DrawStringToHandle(textX + kShadowOffset, textY + kShadowOffset, bossName, 0x000000, m_font);
    DrawStringToHandle(textX, textY, bossName, kColorWhite, m_font);
}

void BossUI::ReloadFonts(float scale) 
{
	m_font.Reload(scale); // スケールに応じてフォントを再生成
}

void BossUI::DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor)
{
    VERTEX2D Vertex[6];
    float fx1 = static_cast<float>(x1);
    float fy1 = static_cast<float>(y1);
    float fx2 = static_cast<float>(x2);
    float fy2 = static_cast<float>(y2);

    unsigned char topR = (topColor >> 16) & 0xFF;
    unsigned char topG = (topColor >> 8) & 0xFF;
    unsigned char topB = topColor & 0xFF;
    
    unsigned char btmR = (bottomColor >> 16) & 0xFF;
    unsigned char btmG = (bottomColor >> 8) & 0xFF;
    unsigned char btmB = bottomColor & 0xFF;

    // 左上
    Vertex[0].pos = VGet(fx1, fy1, 0.0f); Vertex[0].rhw = 1.0f; Vertex[0].u = 0.0f; Vertex[0].v = 0.0f; Vertex[0].dif = GetColorU8(topR, topG, topB, 255);
    // 右上
    Vertex[1].pos = VGet(fx2, fy1, 0.0f); Vertex[1].rhw = 1.0f; Vertex[1].u = 0.0f; Vertex[1].v = 0.0f; Vertex[1].dif = GetColorU8(topR, topG, topB, 255);
    // 左下
    Vertex[2].pos = VGet(fx1, fy2, 0.0f); Vertex[2].rhw = 1.0f; Vertex[2].u = 0.0f; Vertex[2].v = 0.0f; Vertex[2].dif = GetColorU8(btmR, btmG, btmB, 255);
    
    // 左下
    Vertex[3] = Vertex[2];
    // 右上
    Vertex[4] = Vertex[1];
    // 右下
    Vertex[5].pos = VGet(fx2, fy2, 0.0f); Vertex[5].rhw = 1.0f; Vertex[5].u = 0.0f; Vertex[5].v = 0.0f; Vertex[5].dif = GetColorU8(btmR, btmG, btmB, 255);

    DrawPolygon2D(Vertex, 2, DX_NONE_GRAPH, true);
}
