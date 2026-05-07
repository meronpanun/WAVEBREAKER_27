#pragma once
#include "SafeHandle.h"
#include <string>

/// <summary>
/// フォントリソースを管理し、スケール変更に対応する機能を提供するクラス
/// </summary>
class ManagedFont
{
public:
    /// <summary>
    /// 既定のコンストラクタ
    /// </summary>
    ManagedFont() : m_baseSize(0), m_thick(0), m_fontType(0) {}

    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="fontName">フォント名</param>
    /// <param name="baseSize">基準サイズ</param>
    /// <param name="thick">太さ</param>
    /// <param name="fontType">フォントタイプ</param>
    /// <param name="initialScale">初期スケール (デフォルト: 1.0f)</param>
    ManagedFont(const std::string& fontName, int baseSize, int thick, int fontType, float initialScale = 1.0f);
    ~ManagedFont() = default;

    /// <summary>
    /// フォントをロードする
    /// </summary>
    void Load(const std::string& fontName, int baseSize, int thick, int fontType, float initialScale = 1.0f);

    /// <summary>
    /// 指定されたスケールでフォントを再作成する
    /// </summary>
    void Reload(float scale);

    /// <summary>
    /// 現在のハンドルを取得
    /// </summary>
    int Get() const { return m_handle.Get(); }

    /// <summary>
    /// int型への暗黙変換
    /// </summary>
    operator int() const { return m_handle.Get(); }

    bool IsValid() const { return m_handle.IsValid(); }

private:
    SafeHandle<FontDeleter> m_handle;
    std::string m_fontName;
    int m_baseSize;
    int m_thick;
    int m_fontType;
};
