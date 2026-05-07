#include "DirectionIndicator.h"
#include "Player.h"
#include "EnemyBase.h"
#include "Game.h"
#include "Camera.h"
#include <DxLib.h>
#include <cassert>

namespace
{
	// インジケーターの描画半径
	constexpr float kIndicatorRadius = 150.0f;
	// インジケーターの表示時間
    constexpr float kDisplayDuration = 2.0f;
	// フレームレート
	constexpr float kFrameRate = 60.0f;
}

int DirectionIndicator::s_indicatorImage = -1;

DirectionIndicator::DirectionIndicator() 
    : m_pPlayer(nullptr)
{
}

DirectionIndicator::~DirectionIndicator()
{
}

void DirectionIndicator::LoadResources()
{
    s_indicatorImage = LoadGraph("data/image/DirectionIndicator.png");
    assert(s_indicatorImage != -1);
}

void DirectionIndicator::DeleteResources()
{
    DeleteGraph(s_indicatorImage);
}

void DirectionIndicator::Init(Player* player)
{
    m_pPlayer = player;
}

void DirectionIndicator::Update(const std::vector<std::shared_ptr<EnemyBase>>& enemies)
{
    // プレイヤーが存在しない場合は処理しない
	if (!m_pPlayer) return; 

    // 攻撃された敵の表示時間を更新
    for (auto it = m_attackedEnemies.begin(); it != m_attackedEnemies.end();)
    {
        it->displayTime -= 1.0f / kFrameRate; // フレーム時間を減算
        
        // 表示時間が終了した敵を削除
        if (it->displayTime <= 0.0f)
        {
            it = m_attackedEnemies.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void DirectionIndicator::Draw()
{
    // プレイヤーが存在しないか、攻撃された敵がいない場合は描画しない
	if (!m_pPlayer || m_attackedEnemies.empty() || s_indicatorImage == -1) return;

	// カメラの取得
    const auto& camera = m_pPlayer->GetCamera();
    if (!camera) return; // カメラがない場合は処理なし

    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);
    float centerX = screenW * 0.5f;
    float centerY = screenH * 0.5f;

    // プレイヤーの前方ベクトル
    Vec3 playerForward = VSub(camera->GetTarget(), camera->GetPos());
    playerForward.y = 0;
    playerForward.Normalize();

	// 攻撃された敵に対してインジケーターを描画
    for (const auto& attackedEnemy : m_attackedEnemies)
    {        
        // プレイヤーから敵への方向ベクトル
        Vec3 dirToEnemy = attackedEnemy.position - Vec3(m_pPlayer->GetPos());
        dirToEnemy.y = 0;
        if (dirToEnemy.Length() < 0.001f) continue; // プレイヤーと敵が同じ位置の場合はスキップ
		dirToEnemy.Normalize();

        // プレイヤー前方ベクトルと敵への方向ベクトルのなす角度を計算
		// atan2を使用して、-πからπの範囲で角度を取得
        float angle = atan2(dirToEnemy.x * playerForward.z - dirToEnemy.z * playerForward.x,  
                              dirToEnemy.x * playerForward.x + dirToEnemy.z * playerForward.z);

        // インジケーターの位置を計算
		// 画面中心からkIndicatorRadiusの距離に配置
        float indicatorX = centerX + kIndicatorRadius * sin(angle); 
        float indicatorY = centerY - kIndicatorRadius * cos(angle);

        // 表示時間に応じてアルファ値を調整（フェードアウト効果）
        float alpha = attackedEnemy.displayTime / attackedEnemy.maxDisplayTime;
        alpha = (std::max)(0.0f, (std::min)(1.0f, alpha)); // 0.0～1.0の範囲に制限
        
        // アルファブレンドを設定
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha * 255));
        
        // インジケーターを描画
        DrawRotaGraphF(indicatorX, indicatorY, 0.1, angle, s_indicatorImage, true);
        
        // ブレンドモードを元に戻す
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void DirectionIndicator::ShowAttackedEnemyDirection(const Vec3& enemyPos)
{
    // 攻撃された敵の情報を追加
    m_attackedEnemies.emplace_back(enemyPos, kDisplayDuration);
}