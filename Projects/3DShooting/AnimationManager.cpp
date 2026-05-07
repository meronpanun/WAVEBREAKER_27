#include "AnimationManager.h"
#include "EnemyBase.h"
#include <cassert>

AnimationManager::AnimationManager()
{
}

AnimationManager::~AnimationManager()
{
    // マネージャーが破棄される際に全てのアニメーションをデタッチ
    for (const auto& pair : m_attachedAnimHandles)
    {
        MV1DetachAnim(pair.first, 0);
    }
}

// モデルハンドルとアニメーション名からアニメーションインデックスを取得
int AnimationManager::GetAnimIndexInternal(int modelHandle, const std::string& animName)
{
    // キャッシュをチェック
    if (m_animIndexesCache.count(modelHandle) && m_animIndexesCache[modelHandle].count(animName))
    {
        return m_animIndexesCache[modelHandle][animName];
    }

    // キャッシュになければDxLibから取得し、キャッシュする
    int animIndex = MV1GetAnimIndex(modelHandle, animName.c_str());
    if (animIndex != -1)
    {
        m_animIndexesCache[modelHandle][animName] = animIndex;
    }
    return animIndex;
}

// モデルにアニメーションをアタッチして再生
float AnimationManager::PlayAnimation(int modelHandle, const std::string& animName, bool loop)
{
    // 既に何かアタッチされている場合はデタッチ
    if (m_attachedAnimHandles.count(modelHandle) && m_attachedAnimHandles[modelHandle] != -1)
    {
        MV1DetachAnim(modelHandle, 0);
        m_attachedAnimHandles[modelHandle] = -1;
    }

	int animIndex = GetAnimIndexInternal(modelHandle, animName); // インデックスを取得

	// アニメーションが見つかればアタッチして再生
    if (animIndex != -1)
    {
        int attachedHandle = MV1AttachAnim(modelHandle, animIndex, -1, loop);
        m_attachedAnimHandles[modelHandle] = attachedHandle;
        if (attachedHandle != -1)
        {
            float totalTime = MV1GetAnimTotalTime(modelHandle, attachedHandle);
            m_currentAnimTotalTimes[modelHandle] = totalTime;
            MV1SetAttachAnimTime(modelHandle, 0, 0.0f); // アニメーション開始時間をリセット
            return totalTime;
        }
    }

    m_attachedAnimHandles[modelHandle]   = -1;   // アニメーションが見つからない場合は無効なハンドルを設定
	m_currentAnimTotalTimes[modelHandle] = 0.0f; // 総時間も0に設定 
    return 0;
}

// アニメーションの再生時間を更新
void AnimationManager::UpdateAnimationTime(int modelHandle, float animTime)
{
	// アタッチされているアニメーションがあれば時間を更新
    if (m_attachedAnimHandles.count(modelHandle) && m_attachedAnimHandles[modelHandle] != -1)
    {
		MV1SetAttachAnimTime(modelHandle, 0, animTime); // アニメーション時間を更新
    }
}

// 指定アニメーションの総時間を取得
float AnimationManager::GetAnimationTotalTime(int modelHandle, const std::string& animName) const
{
    // 指定アニメーション名のインデックスを取得
    int animIndex = MV1GetAnimIndex(modelHandle, animName.c_str());
    if (animIndex != -1)
    {
        return MV1GetAnimTotalTime(modelHandle, animIndex);
    }

    // 現在アタッチされていない、またはキャッシュにない場合は0を返す
    return 0;
}

// 現在アタッチされているアニメーションのハンドルを取得
int AnimationManager::GetCurrentAttachedAnimHandle(int modelHandle) const
{
    if (m_attachedAnimHandles.count(modelHandle))
    {
        return m_attachedAnimHandles.at(modelHandle);
    }
    return -1;
}

// 現在アタッチされているアニメーションのハンドルをリセット
void AnimationManager::ResetAttachedAnimHandle(int modelHandle)
{
    if (m_attachedAnimHandles.count(modelHandle))
    {
        m_attachedAnimHandles[modelHandle] = -1;
    }
}

// 状態に対応するアニメーションを再生
float AnimationManager::PlayState(int modelHandle, EnemyBase::AnimState state, bool loop)
{
	// 状態に対応するアニメーション名を取得
    auto it = m_animStateToAnimName.find(state);

    // アニメーション名が見つからない場合は0を返す
	if (it == m_animStateToAnimName.end()) return 0; 

	// アニメーションを再生
    float total = PlayAnimation(modelHandle, it->second, loop);

	m_modelCurrentState[modelHandle] = state; // 現在の状態を更新
	m_modelAnimTime[modelHandle]     = 0.0f;  // アニメーション時間をリセット
	return total; // アニメーションの総時間を返す
}

void AnimationManager::Update(int modelHandle, float delta)
{
	// アニメーション時間を更新
    if (m_modelAnimTime.count(modelHandle)) 
    {
        m_modelAnimTime[modelHandle] += delta;
        UpdateAnimationTime(modelHandle, m_modelAnimTime[modelHandle]);
    }
}

// アニメーションが終了したかどうかを判定
bool AnimationManager::IsAnimationFinished(int modelHandle) const
{
    // 状態が記録されていない場合はfalse
	if (!m_modelCurrentState.count(modelHandle)) return false; 

	// 現在の状態に対応するアニメーション名を取得
    EnemyBase::AnimState state = m_modelCurrentState.at(modelHandle);
    auto it = m_animStateToAnimName.find(state);

    if (it == m_animStateToAnimName.end()) return false; 
    float total = 0.0f;

	// アニメーションの総時間を取得
    if (m_currentAnimTotalTimes.count(modelHandle)) 
    {
        total = m_currentAnimTotalTimes.at(modelHandle);
    }
    if (!m_modelAnimTime.count(modelHandle)) return false;
    return m_modelAnimTime.at(modelHandle) >= total && total > 0.0f;
}

// 指定した状態に対応するアニメーション名を設定
void AnimationManager::SetAnimName(EnemyBase::AnimState state, const std::string& animName)
{
	m_animStateToAnimName[state] = animName;
}
