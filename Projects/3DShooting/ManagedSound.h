#pragma once
#include "SafeHandle.h"
#include <string>

/// <summary>
/// 音声リソースを管理するクラス
/// </summary>
class ManagedSound
{
public:
    ManagedSound() = default;

    /// <summary>
    /// パスを指定して音声をロードするコンストラクタ
    /// </summary>
    explicit ManagedSound(const std::string& path);

    ~ManagedSound() = default;

    /// <summary>
    /// 音声をロードする。既にロード済みの場合は解放してからロードする。
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

    /// <summary>
    /// 再生する wrapper of PlaySoundMem
    /// </summary>
    /// <param name="playType">再生タイプ (DX_PLAYTYPE_NORMALなど)</param>
    /// <param name="topPositionFlag">先頭から再生するかどうか</param>
    void Play(int playType, int topPositionFlag = 1);

    /// <summary>
    /// 停止する wrapper of StopSoundMem
    /// </summary>
    void Stop();

    /// <summary>
    /// 再生中かチェック wrapper of CheckSoundMem
    /// </summary>
    bool IsPlaying() const;

private:
    SafeHandle<SoundDeleter> m_handle;
    std::string m_path;
};
