#include "ManagedSound.h"

ManagedSound::ManagedSound(const std::string& path)
{
    Load(path);
}

void ManagedSound::Load(const std::string& path)
{
    m_path = path;
    int newHandle = LoadSoundMem(path.c_str());
    m_handle.Reset(newHandle);
}

void ManagedSound::Play(int playType, int topPositionFlag)
{
    if (m_handle.IsValid())
    {
        PlaySoundMem(m_handle.Get(), playType, topPositionFlag);
    }
}

void ManagedSound::Stop()
{
    if (m_handle.IsValid())
    {
        StopSoundMem(m_handle.Get());
    }
}

bool ManagedSound::IsPlaying() const
{
    if (m_handle.IsValid())
    {
        return CheckSoundMem(m_handle.Get()) != 0;
    }
    return false;
}
