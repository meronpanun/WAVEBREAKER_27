#pragma once
#include "DxLib.h"
#include <utility>

// デリータ定義
struct FontDeleter { void operator()(int h) { DeleteFontToHandle(h); } };
struct GraphDeleter { void operator()(int h) { DeleteGraph(h); } };
struct SoundDeleter { void operator()(int h) { DeleteSoundMem(h); } };
struct ModelDeleter { void operator()(int h) { MV1DeleteModel(h); } };

/// <summary>
/// リソースハンドルの安全な管理を提供するクラステンプレート
/// </summary>
template <typename Deleter>
class SafeHandle
{
public:
    SafeHandle() : m_handle(-1) {}
    explicit SafeHandle(int handle) : m_handle(handle) {}

    // コピー禁止
    SafeHandle(const SafeHandle&) = delete;
    SafeHandle& operator=(const SafeHandle&) = delete;

    // ムーブ許可
    SafeHandle(SafeHandle&& other) noexcept : m_handle(other.m_handle)
    {
        other.m_handle = -1;
    }

    SafeHandle& operator=(SafeHandle&& other) noexcept
    {
        if (this != &other)
        {
            Reset();
            m_handle = other.m_handle;
            other.m_handle = -1;
        }
        return *this;
    }

    ~SafeHandle()
    {
        Reset();
    }

    /// <summary>
    /// 現在のハンドルを解放し、新しいハンドルを設定する
    /// </summary>
    void Reset(int newHandle = -1)
    {
        // 自己代入チェックも兼ねて、異なる場合のみ処理
        if (m_handle != -1 && m_handle != newHandle)
        {
            Deleter()(m_handle);
        }
        m_handle = newHandle;
    }

    /// <summary>
    /// ハンドル値を取得する
    /// </summary>
    int Get() const { return m_handle; }

    /// <summary>
    /// int型への暗黙変換（DxLib関数への渡しやすさのため）
    /// </summary>
    operator int() const { return m_handle; }

    /// <summary>
    /// ハンドルが有効かどうか (-1でないか)
    /// </summary>
    bool IsValid() const { return m_handle != -1; }

private:
    int m_handle;
};
