#pragma once
#include "SafeHandle.h"
#include <string>

/// <summary>
/// 画像リソースを管理するクラス
/// </summary>
class ManagedGraph
{
public:
    ManagedGraph() = default;
    
    /// <summary>
    /// パスを指定して画像をロードするコンストラクタ
    /// </summary>
    explicit ManagedGraph(const std::string& path);
    
    ~ManagedGraph() = default;

    /// <summary>
    /// 画像をロードする。既にロード済みの場合は解放してからロードする。
    /// </summary>
    void Load(const std::string& path);

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
    SafeHandle<GraphDeleter> m_handle;
    std::string m_path;
};
