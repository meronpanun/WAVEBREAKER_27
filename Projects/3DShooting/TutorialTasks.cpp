#include "TutorialTasks.h"
#include "WaveManager.h"
#include "Player.h"
#include "TaskTutorialManager.h"


void ShootTutorialTask::Start(WaveManager* pWaveManager, Player* pPlayer)
{
    if (pWaveManager) pWaveManager->SpawnTutorialWave(1);
    if (pPlayer) pPlayer->SetAttackRestrictions(AttackType::Shoot);
}

void ShootTutorialTask::Update() {}

void ShootTutorialTask::DrawTaskUI(int x, int y, float scale, int alpha, const TaskTutorialManager* pManager)
{
    int currentX = x;
    int spacing = static_cast<int>(8 * scale);
    int diamondSize = static_cast<int>(30 * scale);
    int mouseImgSize = static_cast<int>(36 * scale);

    // ダイアモンドアイコン
    DrawExtendGraph(currentX, y, currentX + diamondSize, y + diamondSize, pManager->GetDiamondImg(), true);
    currentX += diamondSize + spacing;

    // マウス左クリックアイコン
    DrawExtendGraph(currentX, y, currentX + mouseImgSize, y + mouseImgSize, pManager->GetMouseLeftImg(), true);
    currentX += mouseImgSize + spacing;

    DrawStringToHandle(currentX, y, "でゾンビを倒す", 0xFFFFFF, pManager->GetTaskFont());

    // 武器切り替えヒント
    if (alpha >= 200)
    {
        int hintY = y + static_cast<int>(80 * scale);
        int hintX = x;
        DrawStringToHandle(hintX, hintY, "武器切り替え: ", 0xFFFFFF, pManager->GetTaskFont());
        hintX += GetDrawStringWidthToHandle("武器切り替え: ", -1, pManager->GetTaskFont());

        DrawExtendGraph(hintX, hintY, hintX + mouseImgSize, hintY + mouseImgSize, pManager->GetAlpha1Img(), true);
        hintX += mouseImgSize + spacing;
        DrawExtendGraph(hintX, hintY, hintX + mouseImgSize, hintY + mouseImgSize, pManager->GetAlpha2Img(), true);
        hintX += mouseImgSize + spacing;
        DrawStringToHandle(hintX, hintY, " / ", 0xFFFFFF, pManager->GetTaskFont());
        hintX += GetDrawStringWidthToHandle(" / ", -1, pManager->GetTaskFont());
        DrawExtendGraph(hintX, hintY, hintX + mouseImgSize, hintY + mouseImgSize, pManager->GetMouseWheelImg(), true);
    }
}

bool ShootTutorialTask::IsCompleted() const { return m_kills >= kGoal; }
float ShootTutorialTask::GetProgress() const { return static_cast<float>(m_kills) / kGoal; }
std::string ShootTutorialTask::GetProgressText() const { return std::to_string(m_kills) + "/" + std::to_string(kGoal); }
void ShootTutorialTask::NotifyEnemyKilled(int attackType)
{
    if (attackType == (int)AttackType::Shoot || attackType == (int)AttackType::Shotgun) m_kills++;
}

// --- TackleTutorialTask ---
void TackleTutorialTask::Start(WaveManager* pWaveManager, Player* pPlayer)
{
    if (pWaveManager) pWaveManager->SpawnTutorialWave(2);
    if (pPlayer) pPlayer->SetAttackRestrictions(AttackType::Tackle);
}

void TackleTutorialTask::Update() {}

void TackleTutorialTask::DrawTaskUI(int x, int y, float scale, int alpha, const TaskTutorialManager* pManager)
{
    int currentX = x;
    int spacing = static_cast<int>(8 * scale);
    int diamondSize = static_cast<int>(30 * scale);
    int mouseImgSize = static_cast<int>(36 * scale);

    DrawExtendGraph(currentX, y, currentX + diamondSize, y + diamondSize, pManager->GetDiamondImg(), true);
    currentX += diamondSize + spacing;

    DrawExtendGraph(currentX, y, currentX + mouseImgSize, y + mouseImgSize, pManager->GetMouseRightImg(), true);
    currentX += mouseImgSize + spacing;
    DrawStringToHandle(currentX, y, "長押し", 0xFFFFFF, pManager->GetTaskFont());
    currentX += GetDrawStringWidthToHandle("長押し", -1, pManager->GetTaskFont()) + spacing;

    DrawExtendGraph(currentX, y, currentX + mouseImgSize, y + mouseImgSize, pManager->GetLockOnUIImg(), true);
    currentX += mouseImgSize + spacing;

    DrawExtendGraph(currentX, y, currentX + mouseImgSize, y + mouseImgSize, pManager->GetMouseLeftImg(), true);
    currentX += mouseImgSize + spacing;
    DrawStringToHandle(currentX, y, "でタックル", 0xFFFFFF, pManager->GetTaskFont());
}

bool TackleTutorialTask::IsCompleted() const { return m_kills >= kGoal; }
float TackleTutorialTask::GetProgress() const { return static_cast<float>(m_kills) / kGoal; }
std::string TackleTutorialTask::GetProgressText() const { return std::to_string(m_kills) + "/" + std::to_string(kGoal); }
void TackleTutorialTask::NotifyEnemyKilled(int attackType)
{
    if (attackType == (int)AttackType::Tackle) m_kills++;
}

// --- ShieldThrowTutorialTask ---
void ShieldThrowTutorialTask::Start(WaveManager* pWaveManager, Player* pPlayer)
{
    if (pWaveManager) pWaveManager->SpawnTutorialWave(3);
    if (pPlayer) pPlayer->SetAttackRestrictions(AttackType::ShieldThrow);
}

void ShieldThrowTutorialTask::Update() {}

void ShieldThrowTutorialTask::DrawTaskUI(int x, int y, float scale, int alpha, const TaskTutorialManager* pManager)
{
    int currentX = x;
    int spacing = static_cast<int>(8 * scale);
    int diamondSize = static_cast<int>(30 * scale);
    int mouseImgSize = static_cast<int>(36 * scale);

    DrawExtendGraph(currentX, y, currentX + diamondSize, y + diamondSize, pManager->GetDiamondImg(), true);
    currentX += diamondSize + spacing;

    DrawExtendGraph(currentX, y, currentX + mouseImgSize, y + mouseImgSize, pManager->GetRKeyImg(), true);
    currentX += mouseImgSize + spacing;

    DrawStringToHandle(currentX, y, "でシールドを破壊し敵を倒す", 0xFFFFFF, pManager->GetTaskFont());
}

bool ShieldThrowTutorialTask::IsCompleted() const { return m_kills >= kGoal; }
float ShieldThrowTutorialTask::GetProgress() const { return static_cast<float>(m_kills) / kGoal; }
std::string ShieldThrowTutorialTask::GetProgressText() const { return std::to_string(m_kills) + "/" + std::to_string(kGoal); }
void ShieldThrowTutorialTask::NotifyEnemyKilled(int attackType) { m_kills++; }
void ShieldThrowTutorialTask::NotifyShieldThrowKill() { m_kills++; }

// --- ParryTutorialTask ---
void ParryTutorialTask::Start(WaveManager* pWaveManager, Player* pPlayer)
{
    if (pWaveManager) pWaveManager->SpawnTutorialWave(4);
    if (pPlayer) pPlayer->SetAttackRestrictions(AttackType::Parry);
}

void ParryTutorialTask::Update() {}

void ParryTutorialTask::DrawTaskUI(int x, int y, float scale, int alpha, const TaskTutorialManager* pManager)
{
    int currentX = x;
    int spacing = static_cast<int>(8 * scale);
    int diamondSize = static_cast<int>(30 * scale);
    int mouseImgSize = static_cast<int>(36 * scale);

    DrawExtendGraph(currentX, y, currentX + diamondSize, y + diamondSize, pManager->GetDiamondImg(), true);
    currentX += diamondSize + spacing;

    DrawStringToHandle(currentX, y, "遠距離攻撃をパリィする", 0xFFFFFF, pManager->GetTaskFont());

    if (alpha >= 200)
    {
        int hintY = y + static_cast<int>(80 * scale);
        int hintX = x;
        DrawStringToHandle(hintX, hintY, "ガード: ", 0xFFFFFF, pManager->GetTaskFont());
        hintX += GetDrawStringWidthToHandle("ガード: ", -1, pManager->GetTaskFont());
        DrawExtendGraph(hintX, hintY, hintX + mouseImgSize, hintY + mouseImgSize, pManager->GetMouseRightImg(), true);
    }
}

bool ParryTutorialTask::IsCompleted() const { return m_parryCount >= kGoal; }
float ParryTutorialTask::GetProgress() const { return static_cast<float>(m_parryCount) / kGoal; }
std::string ParryTutorialTask::GetProgressText() const { return std::to_string(m_parryCount) + "/" + std::to_string(kGoal); }
void ParryTutorialTask::NotifyParrySuccess() { m_parryCount++; }
