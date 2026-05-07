#pragma once
#include "UIBase.h"
#include "DxLib.h"
#include "ManagedGraph.h"

class Player;

/// <summary>
/// レティクルUIクラス
/// </summary>
class ReticleUI : public UIBase
{
public:
    ReticleUI(Player* player);
    virtual ~ReticleUI();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

private:
    Player* m_pPlayer;
    ManagedGraph m_reticleDefault;
    ManagedGraph m_reticleOnTarget;
    ManagedGraph m_dotDefault;
    ManagedGraph m_dotOnTarget;
};
