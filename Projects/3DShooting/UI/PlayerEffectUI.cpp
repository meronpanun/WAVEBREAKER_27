#include "PlayerEffectUI.h"
#include "Player.h"
#include "PlayerEffectManager.h"

PlayerEffectUI::PlayerEffectUI(Player* player)
    : m_pPlayer(player)
{
}

PlayerEffectUI::~PlayerEffectUI()
{
}

void PlayerEffectUI::Init()
{
}

void PlayerEffectUI::Update(float deltaTime)
{
}

void PlayerEffectUI::Draw()
{
    if (m_pPlayer)
    {
        m_pPlayer->GetEffectManager().Draw();
    }
}
