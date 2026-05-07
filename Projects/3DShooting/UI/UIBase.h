#pragma once

/// <summary>
/// UIの基底クラス
/// </summary>
class UIBase
{
public:
    UIBase() : m_isVisible(true) {}
    virtual ~UIBase() {}

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Init() = 0;

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="deltaTime">経過時間</param>
    virtual void Update(float deltaTime) = 0;

    /// <summary>
    /// 描画処理
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// 表示・非表示設定
    /// </summary>
    void SetVisible(bool visible) { m_isVisible = visible; }

    /// <summary>
    /// 表示状態取得
    /// </summary>
    bool IsVisible() const { return m_isVisible; }

protected:
    bool m_isVisible;
};
