#pragma once
#include "UIBase.h"

class Player;

/// <summary>
/// プレイヤーのエフェクト描画UIクラス
/// </summary>
class PlayerEffectUI : public UIBase
{
public:
    PlayerEffectUI(Player* player);
    virtual ~PlayerEffectUI();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

private:
    Player* m_pPlayer;
};
