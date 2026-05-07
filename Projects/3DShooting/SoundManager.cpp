#include "DxLib.h"
#include "SoundManager.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

std::unique_ptr<SoundManager> SoundManager::s_instance;

SoundManager* SoundManager::GetInstance()
{
    if (!s_instance)
    {
        s_instance.reset(new SoundManager());
    }
    return s_instance.get();
}

SoundManager::~SoundManager()
{
    Release();
}

void SoundManager::Load(const std::string& csvPath)
{
    std::ifstream file(csvPath);
    if (!file.is_open())
    {
        assert(false && "SoundList CSVを開けない");
        return;
    }

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line))
    {
        if (isHeader)
        {
            isHeader = false;
            continue;
        }

        std::stringstream ss(line);
        std::string category, name, path, volumeStr;

        std::getline(ss, category, ',');
        std::getline(ss, name, ',');
        std::getline(ss, path, ',');
        std::getline(ss, volumeStr, ',');

        if (category.empty() || name.empty() || path.empty()) continue;

        int handle = LoadSoundMem(path.c_str());
        if (handle != -1)
        {
            int volume = volumeStr.empty() ? 255 : std::stoi(volumeStr);
            ChangeVolumeSoundMem(volume, handle);

            m_soundMap[category][name] = { handle, volume };
        }
        else
        {
            assert(false && "サウンドファイルのロードに失敗");
        }
    }

    file.close();
}

void SoundManager::Play(const std::string& category, const std::string& name, bool loop)
{
    auto itCat = m_soundMap.find(category);
    if (itCat != m_soundMap.end())
    {
        auto itName = itCat->second.find(name);
        if (itName != itCat->second.end())
        {
            // デフォルト音量に戻してから再生
            ChangeVolumeSoundMem(itName->second.defaultVolume, itName->second.handle);
            PlaySoundMem(itName->second.handle, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
        }
    }
}

void SoundManager::Play(const std::string& category, const std::string& name, int volume, bool loop)
{
    auto itCat = m_soundMap.find(category);
    if (itCat != m_soundMap.end())
    {
        auto itName = itCat->second.find(name);
        if (itName != itCat->second.end())
        {
            ChangeVolumeSoundMem(volume, itName->second.handle);
            PlaySoundMem(itName->second.handle, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
        }
    }
}

void SoundManager::Stop(const std::string& category, const std::string& name)
{
    auto itCat = m_soundMap.find(category);
    if (itCat != m_soundMap.end())
    {
        auto itName = itCat->second.find(name);
        if (itName != itCat->second.end())
        {
            StopSoundMem(itName->second.handle);
        }
    }
}

void SoundManager::SetVolume(const std::string& category, const std::string& name, int volume)
{
    auto itCat = m_soundMap.find(category);
    if (itCat != m_soundMap.end())
    {
        auto itName = itCat->second.find(name);
        if (itName != itCat->second.end())
        {
            ChangeVolumeSoundMem(volume, itName->second.handle);
        }
    }
}

int SoundManager::GetHandle(const std::string& category, const std::string& name)
{
    auto itCat = m_soundMap.find(category);
    if (itCat != m_soundMap.end())
    {
        auto itName = itCat->second.find(name);
        if (itName != itCat->second.end())
        {
            return itName->second.handle;
        }
    }
    return -1;
}

void SoundManager::Update(float deltaTime)
{
    if (m_currentBgm.isPlaying || m_currentBgm.isStopping)
    {
        bool isFading = false;
        if (m_currentBgm.currentVolumeRatio < m_currentBgm.targetVolumeRatio)
        {
            m_currentBgm.currentVolumeRatio += m_currentBgm.fadeSpeed * deltaTime;

            if (m_currentBgm.currentVolumeRatio > m_currentBgm.targetVolumeRatio)
            {
                m_currentBgm.currentVolumeRatio = m_currentBgm.targetVolumeRatio;
            }
            isFading = true;
        }
        else if (m_currentBgm.currentVolumeRatio > m_currentBgm.targetVolumeRatio)
        {
            m_currentBgm.currentVolumeRatio -= m_currentBgm.fadeSpeed * deltaTime;

            if (m_currentBgm.currentVolumeRatio < m_currentBgm.targetVolumeRatio)
            {
                m_currentBgm.currentVolumeRatio = m_currentBgm.targetVolumeRatio;
            }
            isFading = true;
        }

        if (isFading)
        {
            auto& data = m_soundMap[m_currentBgm.category][m_currentBgm.name];
            int volume = static_cast<int>(data.defaultVolume * m_currentBgm.currentVolumeRatio);
            ChangeVolumeSoundMem(volume, data.handle);
        }

        // 停止完了時
        if (m_currentBgm.isStopping && m_currentBgm.currentVolumeRatio <= 0.0f)
        {
            auto& data = m_soundMap[m_currentBgm.category][m_currentBgm.name];
            StopSoundMem(data.handle);
            m_currentBgm.isPlaying = false;
            m_currentBgm.isStopping = false;
            m_currentBgm.category = "";
            m_currentBgm.name = "";
        }
    }
}

void SoundManager::PlayBGM(const std::string& category, const std::string& name, bool loop, float fadeTime)
{
    // 同じBGMが既に再生中の場合は何もしない
    if (m_currentBgm.isPlaying && m_currentBgm.category == category && m_currentBgm.name == name && !m_currentBgm.isStopping) return;

    // 別のBGMが再生中の場合は一度停止（フェードアウト）してから再生する
    // ただし今回はシンプルにするため、即座に新しいBGMのフェードインを開始する
    if (m_currentBgm.isPlaying)
    {
        auto& oldData = m_soundMap[m_currentBgm.category][m_currentBgm.name];
        StopSoundMem(oldData.handle);
    }

    auto itCat = m_soundMap.find(category);
    if (itCat != m_soundMap.end())
    {
        auto itName = itCat->second.find(name);
        if (itName != itCat->second.end())
        {
            m_currentBgm.category = category;
            m_currentBgm.name = name;
            m_currentBgm.isPlaying = true;
            m_currentBgm.isStopping = false;
            m_currentBgm.targetVolumeRatio = 1.0f;
            
            if (fadeTime > 0.0f)
            {
                m_currentBgm.currentVolumeRatio = 0.0f;
                m_currentBgm.fadeSpeed = 1.0f / fadeTime;
                ChangeVolumeSoundMem(0, itName->second.handle);
            }
            else
            {
                m_currentBgm.currentVolumeRatio = 1.0f;
                m_currentBgm.fadeSpeed = 0.0f;
                ChangeVolumeSoundMem(itName->second.defaultVolume, itName->second.handle);
            }

            PlaySoundMem(itName->second.handle, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
        }
    }
}

void SoundManager::StopBGM(float fadeTime)
{
    if (!m_currentBgm.isPlaying) return;

    if (fadeTime > 0.0f)
    {
        m_currentBgm.targetVolumeRatio = 0.0f;
        m_currentBgm.fadeSpeed = 1.0f / fadeTime;
        m_currentBgm.isStopping = true;
    }
    else
    {
        auto& data = m_soundMap[m_currentBgm.category][m_currentBgm.name];
        StopSoundMem(data.handle);
        m_currentBgm.isPlaying = false;
        m_currentBgm.category = "";
        m_currentBgm.name = "";
    }
}

void SoundManager::StopAllSE()
{
    // 1. 登録されている全ハンドルを個別に停止（BGM含む）
    for (auto& categoryPair : m_soundMap)
    {
        for (auto& soundPair : categoryPair.second)
        {
            StopSoundMem(soundPair.second.handle);
        }
    }

    // 2. DXライブラリが管理するすべてのサウンドハンドルを念のため一括停止
    StopSoundMem(-1);
    
    // 3. BGMの状態管理を完全に初期化
    m_currentBgm.isPlaying = false;
    m_currentBgm.isStopping = false;
    m_currentBgm.category = "";
    m_currentBgm.name = "";
    m_currentBgm.currentVolumeRatio = 0.0f;
    m_currentBgm.targetVolumeRatio = 0.0f;
    m_currentBgm.fadeSpeed = 0.0f;
}

void SoundManager::Release()
{
    for (auto& category : m_soundMap)
    {
        for (auto& sound : category.second)
        {
            DeleteSoundMem(sound.second.handle);
        }
    }
    m_soundMap.clear();
}
