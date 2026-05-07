#include "ManagedFont.h"

ManagedFont::ManagedFont(const std::string& fontName, int baseSize, int thick, int fontType, float initialScale)
    : m_fontName(fontName)
    , m_baseSize(baseSize)
    , m_thick(thick)
    , m_fontType(fontType)
{
    Reload(initialScale);
}

void ManagedFont::Load(const std::string& fontName, int baseSize, int thick, int fontType, float initialScale)
{
    m_fontName = fontName;
    m_baseSize = baseSize;
    m_thick = thick;
    m_fontType = fontType;
    Reload(initialScale);
}

void ManagedFont::Reload(float scale)
{
    // フォント情報が未設定の場合はスキップ
    if (m_fontName.empty() || m_baseSize <= 0) return;

    // スケール後のサイズを計算
    int scaledSize = static_cast<int>(m_baseSize * scale);
    if (scaledSize <= 0) return;

    // フォントを作成し、SafeHandleにセット（古いハンドルは自動解放される）
    int newHandle = CreateFontToHandle(m_fontName.c_str(), scaledSize, m_thick, m_fontType);
    m_handle.Reset(newHandle);
}
