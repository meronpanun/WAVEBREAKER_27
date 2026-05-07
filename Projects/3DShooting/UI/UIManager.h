#pragma once
#include <vector>
#include <memory>
#include "UIBase.h"

/// <summary>
/// UIを一括管理するクラス
/// </summary>
class UIManager
{
public:
    UIManager();
    ~UIManager();

    /// <summary>
    /// UI要素を追加する
    /// </summary>
    void AddUI(std::shared_ptr<UIBase> ui);
    void Init();

    /// <summary>
    /// 全てのUIを更新する
    /// </summary>
    void Update(float deltaTime);

    /// <summary>
    /// 全てのUIを描画する
    /// </summary>
    void Draw();

    /// <summary>
    /// 全てのUIをクリアする
    /// </summary>
    void Clear();

    /// <summary>
    /// 指定した型のUIを取得する（最初に見つかったもの）
    /// </summary>
    template<typename T>
    std::shared_ptr<T> GetUI()
    {
        for (auto& ui : m_uiElements)
        {
            auto casted = std::dynamic_pointer_cast<T>(ui);
            if (casted) return casted;
        }
        return nullptr;
    }

private:
    std::vector<std::shared_ptr<UIBase>> m_uiElements;
};
