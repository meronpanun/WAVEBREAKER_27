#pragma once
#include <string>
#include <vector>
#include <functional>

/// <summary>
/// デバッグメニューを管理するクラス
/// </summary>
class DebugMenu
{
public:
    /// <summary>
    /// メニューアイテムの構造体
    /// </summary>
    struct MenuItem
    {
        std::string name;
        std::vector<MenuItem> children;
        std::function<void()> action;
        std::function<std::string()> stateTextGetter; // 状態表示用のテキストを取得する関数
        bool isOpen = false;
    };

    DebugMenu();
    ~DebugMenu();

    void Update();
    void Draw(int x, int y);

private:
    /// <summary>
    /// メニューアイテムを再帰的に描画する関数
    /// </summary>
    /// <param name="item">描画するメニューアイテム</param>
    /// <param name="x">x座標</param>
    /// <param name="y">y座標</param>
    /// <param name="depth">階層の深さ</param>
    /// <param name="currentPath">現在のメニューアイテムのパス</param>
    /// <param name="selectedPath">選択されているメニューアイテムのパス</param>
    /// <param name="mouseX">x座標のマウス位置</param>
    /// <param name="mouseY">y座標のマウス位置</param>
    /// <param name="leftClicked">左クリックされたかどうか</param>
    void DrawItem(MenuItem& item, int& x, int& y, int depth, const std::vector<int>& currentPath, const std::vector<int>& selectedPath, int mouseX, int mouseY, bool leftClicked);

    /// <summary>
    /// メニューの入力処理を行う
    /// </summary>
    void HandleInput();

    /// <summary>
    /// 現在選択されているメニューアイテムを取得する
    /// </summary>
    /// <returns>選択されているメニューアイテムのポインタ</returns>
    MenuItem* GetSelectedItem();

    // メニュー構築用ヘルパーメソッド
    MenuItem CreatePlayerMenu();
    MenuItem CreateEnemyMenu();
    MenuItem CreateSceneMenu();
    MenuItem CreateItemMenu();
    MenuItem CreateScreenMenu();
    MenuItem CreateStageMenu();

    MenuItem m_root;
    std::vector<int> m_selectedPath;
};

