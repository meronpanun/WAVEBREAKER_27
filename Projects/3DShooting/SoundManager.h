#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

/// <summary>
/// サウンド管理クラス
/// </summary>
class SoundManager
{
public:
    /// <summary>
    /// シングルトンインスタンスの取得
    /// </summary>
    static SoundManager* GetInstance();

    /// <summary>
    /// CSVファイルからSEリストをロードする
    /// </summary>
    /// <param name="csvPath">CSVファイルのパス</param>
    void Load(const std::string& csvPath);

    /// <summary>
    /// SEを再生する
    /// </summary>
    /// <param name="category">カテゴリ名</param>
    /// <param name="name">SE名</param>
    /// <param name="loop">ループ再生フラグ</param>
    void Play(const std::string& category, const std::string& name, bool loop = false);

    /// <summary>
    /// 指定した音量でSEを再生する
    /// </summary>
    /// <param name="category">カテゴリ名</param>
    /// <param name="name">SE名</param>
    /// <param name="volume">音量 (0〜255)</param>
    /// <param name="loop">ループ再生フラグ</param>
    void Play(const std::string& category, const std::string& name, int volume, bool loop = false);

    /// <summary>
    /// SEを停止する
    /// </summary>
    /// <param name="category">カテゴリ名</param>
    /// <param name="name">SE名</param>
    void Stop(const std::string& category, const std::string& name);

    /// <summary>
    /// SEの音量を設定する
    /// </summary>
    /// <param name="category">カテゴリ名</param>
    /// <param name="name">SE名</param>
    /// <param name="volume">音量 (0〜255)</param>
    void SetVolume(const std::string& category, const std::string& name, int volume);

    /// <summary>
    /// SEのハンドルを取得する
    /// </summary>
    /// <param name="category">カテゴリ名</param>
    /// <param name="name">SE名</param>
    /// <returns>DXLibサウンドハンドル</returns>
    int GetHandle(const std::string& category, const std::string& name);

    /// <summary>
    /// 更新処理（BGMのフェード処理など）
    /// </summary>
    /// <param name="deltaTime">デルタタイム</param>
    void Update(float deltaTime);

    /// <summary>
    /// BGMを再生する（既存のBGMがある場合はフェードで切り替える）
    /// </summary>
    /// <param name="category">カテゴリ名</param>
    /// <param name="name">BGM名</param>
    /// <param name="loop">ループするか</param>
    /// <param name="fadeTime">フェード時間（秒）</param>
    void PlayBGM(const std::string& category, const std::string& name, bool loop = true, float fadeTime = 1.0f);

    /// <summary>
    /// BGMを停止する
    /// </summary>
    /// <param name="fadeTime">フェードアウト時間（秒）</param>
    void StopBGM(float fadeTime = 1.0f);

    /// <summary>
    /// 再生中の全てのSEを停止する
    /// </summary>
    void StopAllSE();

    /// <summary>
    /// 全てのSEリソースを解放する
    /// </summary>
    void Release();

    ~SoundManager();
private:
    SoundManager() = default;

    struct SoundData {
        int handle;
        int defaultVolume;
    };

    // Category -> Name -> SoundData
    std::map<std::string, std::map<std::string, SoundData>> m_soundMap;

    // BGM管理用
    struct BgmState {
        std::string category;
        std::string name;
        float currentVolumeRatio; // 0.0f 〜 1.0f
        float targetVolumeRatio;
        float fadeSpeed;
        bool isPlaying;
        bool isStopping;
    };
    BgmState m_currentBgm = { "", "", 0.0f, 0.0f, 0.0f, false, false };

    static std::unique_ptr<SoundManager> s_instance;
};
