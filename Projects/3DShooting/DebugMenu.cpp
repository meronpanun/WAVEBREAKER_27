#include "DebugMenu.h"
#include "CollisionGrid.h"
#include "DxLib.h"
#include "Game.h"
#include "ItemBase.h"
#include "Player.h"
#include "SceneGameOver.h"
#include "SceneMain.h"
#include "SceneManager.h"
#include "SceneResult.h"
#include "SceneTitle.h"
#include "Stage.h"
#include "WaveManager.h"
#include "EnemyNormal.h"
#include "EnemyRunner.h"
#include "EnemyBoss.h"
#include "EnemyAcid.h"
#include "TaskTutorialManager.h"
#include <cassert>


namespace
{
    // UI定数
    constexpr int kIndentWidth = 20;         // インデント幅
    constexpr int kItemWidth = 200;          // アイテムの幅
    constexpr int kItemHeight = 15;          // アイテムの高さ
    constexpr int kTextLineSpacing = 20;     // テキスト行間隔
    constexpr int kIndicatorOffsetX = -10;   // インジケーターのXオフセット
    constexpr int kIndicatorTextOffset = 10; // インジケーターとテキストの間隔
    constexpr int kIndicatorOffsetY = 14;    // インジケーターのYオフセット
    constexpr float kIndicatorClosedAngle = -DX_PI_F * 0.5f; // 90度時計回り
}

DebugMenu::DebugMenu()
{
    // メニュー構造の定義
    m_root.name = "Root";
    m_root.children = {
        {"Character",
         {CreatePlayerMenu(), CreateEnemyMenu()},
         nullptr},
        CreateSceneMenu(),
        CreateItemMenu(),
        CreateScreenMenu(),
        CreateStageMenu()
    };
    m_root.action = nullptr;

    // 初期選択パスを設定（ルートの最初の子を選択）
    if (!m_root.children.empty())
    {
        m_selectedPath.push_back(0);
    }
}

DebugMenu::~DebugMenu()
{
}

void DebugMenu::Update()
{
    HandleInput();
}

void DebugMenu::Draw(int x, int y)
{
    int currentY = y;
    std::vector<int> currentPath;
    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);

    static int prevLeftClick = 0;
    int nowLeftClick = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
    bool leftClicked = (nowLeftClick && !prevLeftClick);

    for (size_t i = 0; i < m_root.children.size(); ++i)
    {
        currentPath = { (int)i };
        DrawItem(m_root.children[i], x, currentY, 0, currentPath, m_selectedPath,
            mouseX, mouseY, leftClicked);
    }

    prevLeftClick = nowLeftClick;
}

void DebugMenu::DrawItem(MenuItem& item, int& x, int& y, int depth,
    const std::vector<int>& currentPath,
    const std::vector<int>& selectedPath, int mouseX,
    int mouseY, bool leftClicked)
{
    bool isSelected = (currentPath == selectedPath);

    int itemX = x + depth * kIndentWidth;
    int itemY = y;
    int itemWidth = kItemWidth;
    int itemHeight = kItemHeight;

    bool isHovered = (mouseX >= itemX && mouseX <= itemX + itemWidth && mouseY >= itemY && mouseY <= itemY + itemHeight);

    if (isHovered && leftClicked)
    {
        m_selectedPath = currentPath;
        isSelected = true;
        if (item.action)
        {
            item.action();
        }
        else if (!item.children.empty())
        {
            item.isOpen = !item.isOpen;
        }
    }

    int color = isHovered ? 0x0000ff : 0xffffff;
    // 子項目がある場合、開閉状態に応じたインジケーター（三角形文字）を描画
    if (!item.children.empty())
    {
        const TCHAR* indicatorChar = _T("▼"); // 下向きの三角形文字を使用
        float rotationAngle = 0.0f;           // 閉じた状態: 右向き

        if (!item.isOpen) // 閉じた状態の場合、右向きに回転
        {
            rotationAngle = kIndicatorClosedAngle; // 90度時計回り
        }

        // 文字のサイズを取得して、回転の中心と描画位置を調整
        int charWidth = GetDrawStringWidth(indicatorChar, 1); // '▼' の幅を取得
        int charHeight = GetFontSize(); // 現在のフォントの高さ

        // 回転の中心を文字の中心に設定
        double rotCenterX = charWidth * 0.5f;
        double rotCenterY = charHeight * 0.5f;

        // 描画位置を計算
        int indicatorDrawX = itemX + kIndicatorOffsetX;
        int indicatorDrawY = itemY + (itemHeight * 0.5f) - (charHeight * 0.5f) + kIndicatorOffsetY;

        // DrawRotaString で回転して描画
        DrawRotaString(indicatorDrawX, indicatorDrawY, 1.0, 1.0, rotCenterX, rotCenterY, rotationAngle, color, 0, false, indicatorChar);
    }

    // テキストの描画
    int textStartX = itemX;
    if (!item.children.empty())
    {
        textStartX += kIndicatorTextOffset; // インジケータの分だけテキストを右にずらす
    }

    std::string displayText = item.name;
    if (item.stateTextGetter)
    {
        displayText += " " + item.stateTextGetter();
    }

    DrawString(textStartX, itemY, displayText.c_str(), color);
    y += kTextLineSpacing;

    if (item.isOpen && !item.children.empty())
    {
        std::vector<int> childPath = currentPath;
        childPath.push_back(0);
        for (size_t i = 0; i < item.children.size(); ++i)
        {
            childPath.back() = i;
            DrawItem(item.children[i], x, y, depth + 1, childPath, selectedPath,
                mouseX, mouseY, leftClicked);
        }
    }
}

void DebugMenu::HandleInput()
{
    static int prevRightClick = 0;
    int nowRightClick = (GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;

    if (nowRightClick && !prevRightClick)
    {
        if (m_selectedPath.size() > 1)
        {
            m_selectedPath.pop_back();
            MenuItem* selected = GetSelectedItem();
            if (selected)
            {
                selected->isOpen = false;
            }
        }
    }

    prevRightClick = nowRightClick;
}

DebugMenu::MenuItem* DebugMenu::GetSelectedItem()
{
    MenuItem* item = &m_root;
    for (int index : m_selectedPath)
    {
        if (index < item->children.size())
        {
            item = &item->children[index];
        }
        else
        {
            return nullptr;
        }
    }
    return item;
}

DebugMenu::MenuItem DebugMenu::CreatePlayerMenu()
{
    MenuItem playerMenu;
    playerMenu.name = "Player";
    playerMenu.children = {
        {"Invincible",
         {},
         [this]() {
           if (Game::m_pPlayer)
           {
             bool isInvincible = !Game::m_pPlayer->IsInvincible();
             Game::m_pPlayer->SetInvincible(isInvincible);
           }
         },
         []() {
           if (Game::m_pPlayer) 
           {
             return Game::m_pPlayer->IsInvincible() ? "[ON]" : "[OFF]";
           }
           return "[N/A]";
         }},
        {"Infinite Ammo",
         {},
         [this]() {
           if (Game::m_pPlayer) 
           {
             bool isInfiniteAmmo = !Game::m_pPlayer->IsInfiniteAmmo();
             Game::m_pPlayer->SetInfiniteAmmo(isInfiniteAmmo);
           }
         },
         []() {
           if (Game::m_pPlayer) 
           {
             return Game::m_pPlayer->IsInfiniteAmmo() ? "[ON]" : "[OFF]";
           }
           return "[N/A]";
         }},
        {"Flight Mode",
         {},
         [this]() {
           if (Game::m_pPlayer) 
           {
             bool isFlightMode = !Game::m_pPlayer->IsFlightMode();
             Game::m_pPlayer->SetFlightMode(isFlightMode);
           }
         },
         []() {
           if (Game::m_pPlayer) 
           {
             return Game::m_pPlayer->IsFlightMode() ? "[ON]" : "[OFF]";
           }
           return "[N/A]";
         }}};
    playerMenu.action = nullptr;
    return playerMenu;
}

DebugMenu::MenuItem DebugMenu::CreateEnemyMenu()
{
    MenuItem enemyMenu;
    enemyMenu.name = "Enemy";
    enemyMenu.children = {
        {"Show Spawn Areas",
         {},
         []() {
           bool isDraw = !WaveManager::IsDrawSpawnAreas();
           WaveManager::SetDrawSpawnAreas(isDraw);
         },
         []() { return WaveManager::IsDrawSpawnAreas() ? "[ON]" : "[OFF]"; }},
        
        {"Collision",
         {{"Normal Enemy",
           {{"Show Collision",
             {},
             []() {
               bool isDraw = !EnemyNormal::ShouldDrawCollision();
               EnemyNormal::SetDrawCollision(isDraw);
             },
             []() { return EnemyNormal::ShouldDrawCollision() ? "[ON]" : "[OFF]"; }},
            {"Shield Collision",
             {},
             []() {
               bool isDraw = !EnemyNormal::IsDrawShieldCollision();
               EnemyNormal::SetDrawShieldCollision(isDraw);
             },
             []() { return EnemyNormal::IsDrawShieldCollision() ? "[ON]" : "[OFF]"; }}},
           nullptr},
          {"Runner Enemy",
           {},
           []() {
             bool isDraw = !EnemyRunner::ShouldDrawCollision();
             EnemyRunner::SetDrawCollision(isDraw);
           },
           []() { return EnemyRunner::ShouldDrawCollision() ? "[ON]" : "[OFF]"; }},
          {"Acid Enemy",
           {},
           []() {
             bool isDraw = !EnemyAcid::ShouldDrawCollision();
             EnemyAcid::SetDrawCollision(isDraw);
           },
           []() { return EnemyAcid::ShouldDrawCollision() ? "[ON]" : "[OFF]"; }},
          {"Boss Enemy",
           {{"Show Collision",
             {},
             []() {
               bool isDraw = !EnemyBoss::ShouldDrawCollision();
               EnemyBoss::SetDrawCollision(isDraw);
             },
             []() {
               return EnemyBoss::ShouldDrawCollision() ? "[ON]" : "[OFF]";
             }},
            {"Attack Hit",
             {},
             []() {
               bool isDraw = !EnemyBoss::IsDrawAttackHit();
               EnemyBoss::SetDrawAttackHit(isDraw);
             },
             []() { return EnemyBoss::IsDrawAttackHit() ? "[ON]" : "[OFF]"; }},
            {"Shield Collision",
             {},
             []() {
               bool isDraw = !EnemyBoss::IsDrawShieldCollision();
               EnemyBoss::SetDrawShieldCollision(isDraw);
             },
             []() { return EnemyBoss::IsDrawShieldCollision() ? "[ON]" : "[OFF]"; }}},
           nullptr}},
         nullptr}
    };
    enemyMenu.action = nullptr;
    return enemyMenu;
}

DebugMenu::MenuItem DebugMenu::CreateSceneMenu()
{
    MenuItem sceneMenu;
    sceneMenu.name = "Scene";
    sceneMenu.children = {
        {"Skip Tutorial",
         {},
         []() { SceneMain::s_isSkipTutorial = !SceneMain::s_isSkipTutorial; },
         []() { return SceneMain::s_isSkipTutorial ? "[ON]" : "[OFF]"; }},
        {"Skip to Parry Tutorial",
         {},
         []() {
             if (TaskTutorialManager::GetInstance()) {
                 TaskTutorialManager::GetInstance()->SkipToParry();
             }
         }},
        {"TitleScene",
         {},
         []() {
           if (Game::m_pSceneManager) {
             Game::m_pSceneManager->RequestChangeScene(new SceneTitle(false));
           }
         }},
        {"MainScene",
         {},
         []() {
           if (Game::m_pSceneManager) {
             Game::m_pSceneManager->RequestChangeScene(new SceneMain(false));
           }
         }},
        {"ResultScene",
         {},
         []() {
           if (Game::m_pSceneManager) {
             Game::m_pSceneManager->RequestChangeScene(new SceneResult());
           }
         }},
        {"GameOverScene",
         {},
         []() {
           if (Game::m_pSceneManager) {
             Game::m_pSceneManager->RequestChangeScene(new SceneGameOver(0, 0, 0));
           }
         }}
    };
    sceneMenu.action = nullptr;
    return sceneMenu;
}

DebugMenu::MenuItem DebugMenu::CreateItemMenu()
{
    MenuItem itemMenu;
    itemMenu.name = "Item";
    itemMenu.children = {
        {"Show Collision",
         {},
         []() {
           bool isDraw = !ItemBase::ShouldDrawCollision();
           ItemBase::SetDrawCollision(isDraw);
         },
         []() { return ItemBase::ShouldDrawCollision() ? "[ON]" : "[OFF]"; }}
    };
    itemMenu.action = nullptr;
    return itemMenu;
}


DebugMenu::MenuItem DebugMenu::CreateScreenMenu()
{
    MenuItem screenMenu;
    screenMenu.name = "Screen";
    screenMenu.children = {
        {"Resolution",
         {{"1280x720",
           {},
           []() { Game::SetResolution(1280, 720); },
           []() {
             return (Game::GetScreenWidth() == 1280 &&
                     Game::GetScreenHeight() == 720)
                        ? "[Current]"
                        : "";
           }},
          {"1920x1080",
           {},
           []() { Game::SetResolution(1920, 1080); },
           []() {
             return (Game::GetScreenWidth() == 1920 &&
                     Game::GetScreenHeight() == 1080)
                        ? "[Current]"
                        : "";
           }}},
         nullptr},
        {"Window Mode",
         {},
         []() {
           bool isWindow = !Game::IsWindowMode();
           Game::SetWindowMode(isWindow);
         },
         []() { return Game::IsWindowMode() ? "[Window]" : "[Fullscreen]"; }},
        {"Show Debug HUD",
         {},
         []() {
           if (SceneMain::Instance()) {
             bool isShow = !SceneMain::Instance()->IsShowDebugHUD();
             SceneMain::Instance()->SetShowDebugHUD(isShow);
           }
         },
         []() {
           if (SceneMain::Instance()) {
             return SceneMain::Instance()->IsShowDebugHUD() ? "[ON]" : "[OFF]";
           }
           return "[N/A]";
         }}
    };
    screenMenu.action = nullptr;
    return screenMenu;
}

DebugMenu::MenuItem DebugMenu::CreateStageMenu()
{
    MenuItem stageMenu;
    stageMenu.name = "Collision";
    stageMenu.children = {
        {"Main Stage Collision",
         {},
         []() {
           bool isDraw = !Stage::ShouldDrawCollision();
           Stage::SetDrawCollision(isDraw);
         },
         []() { return Stage::ShouldDrawCollision() ? "[ON]" : "[OFF]"; }},
        {"Tutorial Stage Collision",
         {},
         []() {
           bool isDraw = !Stage::ShouldDrawTutorialCollision();
           Stage::SetDrawTutorialCollision(isDraw);
         },
         []() {
           return Stage::ShouldDrawTutorialCollision() ? "[ON]" : "[OFF]";
         }},
        {"Spatial Grid Visualization",
         {},
         []() {
           bool isDraw = !CollisionGrid::IsDrawGrid();
           CollisionGrid::SetDrawGrid(isDraw);
         },
         []() { return CollisionGrid::IsDrawGrid() ? "[ON]" : "[OFF]"; }},
        {"Spatial Optimization",
         {},
         []() {
           bool use = !CollisionGrid::IsUseSpatialPartitioning();
           CollisionGrid::SetUseSpatialPartitioning(use);
         },
         []() { return CollisionGrid::IsUseSpatialPartitioning() ? "[ON]" : "[OFF]"; }}
    };
    stageMenu.action = nullptr;
    return stageMenu;
}