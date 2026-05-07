#include "SceneTitle.h"
#include "EffekseerForDXLib.h"
#include "Game.h"
#include "SceneMain.h"
#include "InputManager.h"
#include "DebugUtil.h"
#include "SoundManager.h"
#include <cassert>
#include <random>
#include <memory>

namespace
{
    // タイトルロゴの幅と高さ
    constexpr int kLogoWidth = 1050;
    constexpr int kLogoHeight = 1080;

    // スカイドーム関連
    constexpr float kSkyDomePosY = 60.0f;  // スカイドームのY座標
    constexpr float kSkyDomePosZ = -100.0f; // スカイドームのZ座標
    constexpr float kSkyDomeScale = 150.0f;  // スカイドームのスケール
    constexpr float kSkyDomeRotaSpeed = 0.001f; // スカイドームの回転速度

    // フェード関連
    constexpr int kFadeDuration = 60; // フェードイン・フェードアウトのフレーム数
    constexpr int kWaitDuration = 60; // フェードイン後の待機時間（フレーム数）

    // ゲームスタートテキストの点滅速度
    constexpr int kGameStartTextBlinkSpeed = 4;
}

SceneTitle::SceneTitle(bool isReturningFromOtherScene)
    : m_font("Arial Black", 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_titleLogo("data/image/TitleLogo.png")
    , m_banner("data/image/TitleBanner.png")
    , m_fadeAlpha(0)
    , m_fadeFrame(0)
    , m_sceneFadeAlpha(0)
    , m_waitFrame(0)
    , m_isFadeComplete(false)
    , m_isFadeOut(false)
    , m_isSceneFadeIn(false)
    , m_gameStartTextAlphaDir(1)
    , m_billboardShakeOffsetX(0.0f)
    , m_billboardShakeOffsetY(0.0f)
    , m_billboardShakePower(0.0f)
    , m_shakeTimer(0)
    , m_zombieVoiceTimer(60)
    , m_animIndexIdle(-1)
    , m_animIndexAtkHead(-1)
{
    for (int i = 0; i < 3; ++i) {
        m_fenceShakePower[i] = 0.0f;
    }

    // ロード確認
    assert(m_titleLogo.IsValid());
    assert(m_banner.IsValid());

    // 3Dモデルのロード
    m_skyDome.Reset(MV1LoadModel("data/model/Dome.mv1"));
    m_floorModel.Reset(MV1LoadModel("data/model/RoadFloor.mv1"));
    m_fenceModel.Reset(MV1LoadModel("data/model/MeshFence.mv1"));
    m_armoryBillboardModel.Reset(MV1LoadModel("data/model/ArmoryBillboard.mv1"));
    m_hangarV3Model.Reset(MV1LoadModel("data/model/HangarV3.mv1"));
    m_hangarModel.Reset(MV1LoadModel("data/model/Hangar.mv1"));
    m_containerModel.Reset(MV1LoadModel("data/model/Container.mv1"));
    m_zombieModel.Reset(MV1LoadModel("data/model/NormalZombie.mv1"));

    // スカイドームの設定
    if (m_skyDome.IsValid())
    {
        MV1SetPosition(m_skyDome, VGet(0, kSkyDomePosY, kSkyDomePosZ));
        MV1SetScale(m_skyDome, VGet(kSkyDomeScale, kSkyDomeScale, kSkyDomeScale));
    }

    if (m_zombieModel.IsValid())
    {
        // アニメーションインデックスの取得
        m_animIndexIdle = MV1GetAnimIndex(m_zombieModel, "IDLE");
        m_animIndexAtkHead = MV1GetAnimIndex(m_zombieModel, "ATK_HEAD");

        // ゾンビの初期配置を生成
        std::random_device rd;
        std::mt19937 gen(rd());
        // 画面の端から端までゾンビが湧くように、見えている範囲に合わせて生成範囲を調整（-280.0f 〜 280.0f）
        std::uniform_real_distribution<float> distPosX(-280.0f, 280.0f); 
        // ATK_HEADで前傾姿勢になった時にフェンスを貫通せず、かつ触れているように見える距離に微調整
        std::uniform_real_distribution<float> distPosZ(200.0f, 450.0f);  // 手前を少しフェンス寄りに戻す
        std::uniform_real_distribution<float> distAngleY(DX_PI_F * -0.2f, DX_PI_F * 0.2f); // 手前（カメラ側）を向く角度
        std::uniform_real_distribution<float> distSpeed(0.8f, 1.2f); // 再生速度のばらつき

        // ゾンビを数十体生成（密度を上げるため60体に増加）
        int i = 0;
        int timeoutCounter = 0;
        constexpr float kMinDistance = 25.0f; // ゾンビ同士の最小距離を縮めて密集できるようにする
        
        while (i < 30 && timeoutCounter < 2000)
        {
            timeoutCounter++;
            
            VECTOR potentialPos = VGet(distPosX(gen), 0.0f, distPosZ(gen));
            
            // 既存のゾンビとの距離をチェック
            bool isTooClose = false;
            for (const auto& zombie : m_zombies)
            {
                if (VDot(VSub(potentialPos, zombie.pos), VSub(potentialPos, zombie.pos)) < kMinDistance * kMinDistance)
                {
                    isTooClose = true;
                    break;
                }
            }
            
            if (isTooClose)
            {
                continue; // 距離が近すぎる場合はやり直し
            }

            ZombieData data;
            data.pos = potentialPos;
            data.angleY = distAngleY(gen);
            
            // 手前ギリギリのゾンビだけがフェンスを叩くように調整(250.0f以下)
            // 奥の方にいるゾンビは待機（IDLE）にする
            if (data.pos.z <= 250.0f && m_animIndexAtkHead != -1)
            {
                // 近いゾンビはフェンスを叩く
                data.animIndex = m_animIndexAtkHead;
            }
            else if (m_animIndexIdle != -1)
            {
                // 奥のゾンビは待機
                data.animIndex = m_animIndexIdle;
            }
            else
            {
                data.animIndex = 0; // 万が一取得できなければ0
            }

            // アタッチして総時間を取得し、そこからランダムな開始位置を決める
            int attachIndex = MV1AttachAnim(m_zombieModel, data.animIndex, -1, false);
            data.totalAnimTime = MV1GetAttachAnimTotalTime(m_zombieModel, attachIndex);
            MV1DetachAnim(m_zombieModel, attachIndex); // 時間取得のための一時アタッチ

            std::uniform_real_distribution<float> distTime(0.0f, data.totalAnimTime);
            data.animTime = distTime(gen);
            data.animSpeed = distSpeed(gen);

            m_zombies.push_back(data);
            i++;
        }
    }
}

SceneTitle::~SceneTitle()
{
}

void SceneTitle::Init()
{
}

SceneBase* SceneTitle::Update()
{
    // タイトルロゴのフェードイン処理
    if (!m_isFadeComplete)
    {
        if (m_fadeFrame < kFadeDuration)
        {
            m_fadeAlpha = static_cast<int>(255.0f * (m_fadeFrame / static_cast<float>(kFadeDuration)));
            m_fadeFrame++;
        }
        else
        {
            m_fadeAlpha = 255;
            m_isFadeComplete = true; // フェードインが完了
            m_fadeFrame = 0;    // フェードアウト用にリセット
        }
        return this;
    }

    // フェードイン後の待機時間をカウント
    if (!m_isFadeOut)
    {
        if (m_waitFrame < kWaitDuration)
        {
            m_waitFrame++;
            return this; // 待機時間中はシーン遷移しない
        }
        else
        {
            m_isFadeOut = true; // 待機が完了したらフェードアウト開始
            m_fadeFrame = 0;    // フェードアウト用にリセット
        }
    }

    // フェードアウト処理
    if (m_isFadeOut && m_fadeAlpha > 0)
    {
        if (m_fadeFrame < kFadeDuration)
        {
            m_fadeAlpha = static_cast<int>(255.0f * (1.0f - (m_fadeFrame / static_cast<float>(kFadeDuration))));
            m_fadeFrame++;
        }
        else
        {
            m_fadeAlpha = 0;
            // フェードアウトが完全に終了した状態＝3D背景のみを描画する状態
        }
        return this; // フェードアウト中（完全に消え切るまで）はゾンビの更新や描画への移行を保留するなら return this; を付けるか、Alphaが0かチェックする。
    }

    // BGM再生
    SoundManager::GetInstance()->PlayBGM("BGM", "Title");

    // 「ゲームスタート」文字の点滅処理
    m_gameStartTextAlpha += m_gameStartTextAlphaDir * kGameStartTextBlinkSpeed;
    if (m_gameStartTextAlpha > 255)
    {
        m_gameStartTextAlpha = 255;
        m_gameStartTextAlphaDir = -1;
    }
    else if (m_gameStartTextAlpha < 0)
    {
        m_gameStartTextAlpha = 0;
        m_gameStartTextAlphaDir = 1;
    }

    // マウスの左クリックをチェック
    // デバッグメニューが表示されていない場合のみ有効
    if (!DebugUtil::IsDebugWindowVisible() && InputManager::GetInstance()->IsTriggerMouseLeft())
    {
        // BGMを停止
        SoundManager::GetInstance()->StopBGM();
        SoundManager::GetInstance()->Play("UI", "Confirm");
        return new SceneMain();
    }

    // ゾンビのアニメーション時間を進める (フェードイン完了後のみ)
    if (m_isFadeComplete)
    {
        float scale = 0.3f;
        float kFenceInterval = 157.8f;
        float kXOffset = 995.6f * scale; // = 298.68f

        for (auto& zombie : m_zombies)
        {
            float oldAnimTime = zombie.animTime;
            float newAnimTime = zombie.animTime + zombie.animSpeed;
            
            // ATK_HEAD アニメーションでフェンスを叩くタイミングを検知して揺れを加算
            if (zombie.animIndex == m_animIndexAtkHead)
            {
                // アニメーションの総再生時間の特定の割合を通過したかチェック
                float hitTiming = zombie.totalAnimTime * 0.4f; 
                
                // 今回のフレームでヒットタイミングを跨いだか判定（ループ時のリセット前の純粋な加算値で比較）
                if (oldAnimTime < hitTiming && newAnimTime >= hitTiming)
                {
                    // ゾンビのX座標から一番近い画面内のフェンス(インデックス i = -1, 0, 1)を特定する
                    // フェンスは視覚的に i * kFenceInterval の位置に配置されているため、オフセットは不要
                    float exactIdx = zombie.pos.x / kFenceInterval;
                    int nearest_i = static_cast<int>(roundf(exactIdx));

                    // 画面に映るフェンス（i = -1, 0, 1）のみ揺らし計算を行う
                    if (nearest_i >= -1 && nearest_i <= 1)
                    {
                        // 配列のインデックス(0,1,2)に変換
                        int arrIdx = nearest_i + 1;

                        // 攻撃されたフェンスに強い揺れを加算
                        m_fenceShakePower[arrIdx] += 5.0f;
                        if (m_fenceShakePower[arrIdx] > 18.0f) m_fenceShakePower[arrIdx] = 18.0f;
                        
                        // 隣接するフェンスに余波（少し弱い揺れ）を加算
                        if (arrIdx - 1 >= 0) {
                            m_fenceShakePower[arrIdx - 1] += 2.0f;
                            if (m_fenceShakePower[arrIdx - 1] > 10.0f) m_fenceShakePower[arrIdx - 1] = 10.0f;
                        }
                        if (arrIdx + 1 <= 2) {
                            m_fenceShakePower[arrIdx + 1] += 2.0f;
                            if (m_fenceShakePower[arrIdx + 1] > 10.0f) m_fenceShakePower[arrIdx + 1] = 10.0f;
                        }
                    }
                }
            }

            // 最後にアニメーション時間をループ考慮して更新
            zombie.animTime = newAnimTime;
            if (zombie.animTime >= zombie.totalAnimTime)
            {
                zombie.animTime = fmodf(zombie.animTime, zombie.totalAnimTime);
            }
        }

        m_shakeTimer += 1;

        // フェンスの揺れパワーの減衰 (3枚分)
        for (int i = 0; i < 3; ++i)
        {
            if (m_fenceShakePower[i] > 0.1f)
            {
                m_fenceShakePower[i] *= 0.7f;
            }
            else
            {
                m_fenceShakePower[i] = 0.0f;
            }
        }

        // 看板の揺れは、画面中央のフェンス（インデックス1）に連動させる
        m_billboardShakePower = m_fenceShakePower[1] * 1.2f;
        
        // 看板の揺れ計算
        if (m_billboardShakePower > 0.1f)
        {
            // 規則的なサイン波ではなく、ランダム値を使って細かくガタガタ揺れるようにする
            // フレームごとに+と-をランダムに振る
            float randX = (GetRand(100) / 100.0f) * 2.0f - 1.0f; // -1.0 〜 1.0
            float randY = (GetRand(100) / 100.0f) * 2.0f - 1.0f; // -1.0 〜 1.0

            m_billboardShakeOffsetX = randX * m_billboardShakePower;
            m_billboardShakeOffsetY = randY * m_billboardShakePower;
        }
        else
        {
            m_billboardShakeOffsetX = 0.0f;
            m_billboardShakeOffsetY = 0.0f;
            m_billboardShakePower = 0.0f;
        }

        // スカイドームの回転
        if (m_skyDome.IsValid())
        {
            MV1SetRotationXYZ(m_skyDome, VGet(0, MV1GetRotationXYZ(m_skyDome).y + kSkyDomeRotaSpeed, 0));
        }

        // ゾンビ環境ボイスの再生
        m_zombieVoiceTimer--;
        if (m_zombieVoiceTimer <= 0)
        {
            int randomIndex = 1 + GetRand(3); // 1〜4
            SoundManager::GetInstance()->Play("EnemyNormal", "Voice" + std::to_string(randomIndex));
            
            // 次の再生までの時間をランダムに設定 (1秒〜3秒)
            m_zombieVoiceTimer = 60 + GetRand(120);
        }
    }

    // 何もしなければシーン遷移しない(タイトル画面のまま)
    return this;
}

void SceneTitle::Draw()
{
    // マウスの位置を取得
    Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

    // ----------------------------------------
    // タイトルロゴの描画 (完全に消え切るまで描画)
    // ----------------------------------------
    if (m_fadeAlpha > 0)
    {
        SetUseZBuffer3D(false);
        SetWriteZBuffer3D(false);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawRectExtendGraph(
            0, 0,
            Game::GetScreenWidth(), Game::GetScreenHeight(),
            0, 0,
            kLogoWidth, kLogoHeight,
            m_titleLogo, true
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ----------------------------------------
    // 3D背景 & 2Dテキストの描画 (フェードアウト完了後のみ)
    // ----------------------------------------
    else // m_fadeAlpha == 0 の場合（フェードアウトが完全に終わった状態）
    {
        // カメラの設定（少し見下ろす位置に修正）
        VECTOR camPos = VGet(0.0f, 120.0f, -80.0f);
        VECTOR camTarget = VGet(0.0f, 50.0f, 100.0f); // ゾンビの群れの中心やや上を向く
        SetCameraPositionAndTarget_UpVecY(camPos, camTarget);
        SetCameraNearFar(5.0f, 15000.0f); // スカイドームなどの遠景用

        // Zバッファ設定などを有効化
        SetUseZBuffer3D(true);
        SetWriteZBuffer3D(true);

        // スカイドーム描画（背面の奥に映るようにするため、Zバッファのテストも書き込みも無効にして一番奥に描画する）
        if (m_skyDome.IsValid())
        {
            SetUseZBuffer3D(false);
            SetWriteZBuffer3D(false);
            MV1DrawModel(m_skyDome);
            
            // 描画後、他の3DモデルのためにZバッファを有効に戻す
            SetUseZBuffer3D(true);
            SetWriteZBuffer3D(true);
        }

        // 床描画
        if (m_floorModel.IsValid())
        {
            SetLightEnable(false);
            SetUseLighting(false);

            // 床モデル(RoadFloor)は画像データから幅2000(X:-1000〜1000)、奥行2000(Z:-1000〜1000)
            // したがってScale 1.0f で使用する
            MV1SetScale(m_floorModel, VGet(1.0f, 1.0f, 1.0f));
            MV1SetRotationXYZ(m_floorModel, VGet(0.0f, 0.0f, 0.0f));
            
            // ゾンビが床に埋まらないようにするためのY座標オフセット
            float floorY = -40.0f;
            
            // 幅2000、奥行2000の床を複数枚並べて広大な地面を作る
            for (int z = -1; z <= 3; ++z)
            {
                for (int x = -2; x <= 2; ++x)
                {
                    // 中心を2000間隔に配置することでピッタリ敷き詰める
                    MV1SetPosition(m_floorModel, VGet(x * 2000.0f, floorY, z * 2000.0f));
                    MV1DrawModel(m_floorModel);
                }
            }

            SetLightEnable(true);
            SetUseLighting(true);
        }

        // フェンス描画
        if (m_fenceModel.IsValid())
        {
            // 画像の頂点座標より、高さが約468ある巨大なモデルであるため
            // ゾンビの高さ(約110)に合わせてスケールを0.3倍程度に縮小します。
            float scale = 0.3f;
            MV1SetScale(m_fenceModel, VGet(scale, scale, scale));

            // モデルの面がZ方向（奥）を向いているため、Y軸で90度回転してX方向（横幅）に沿わせます
            MV1SetRotationXYZ(m_fenceModel, VGet(0.0f, 3.141592654f * 0.5f, 0.0f));

            float kFenceInterval = 157.8f;
            float kXOffset = 995.6f * scale;

            // 画面に映るフェンス3枚のみを描画して不要な負荷を削減（手前側 Z=110 に配置）
            // -1 が左, 0 が中央, 1 が右
            for (int i = -1; i <= 1; ++i)
            {
                float offsetX = 0.0f;
                float offsetY = 0.0f;

                // 配列のインデックス(0, 1, 2)に変換して、個別の揺れパワーを取得
                int arrIdx = i + 1;
                float power = m_fenceShakePower[arrIdx];

                if (power > 0.1f)
                {
                    float randX = (GetRand(100) / 100.0f) * 2.0f - 1.0f;
                    float randY = (GetRand(100) / 100.0f) * 2.0f - 1.0f;
                    offsetX = randX * power;
                    offsetY = randY * power;
                }

                MV1SetPosition(m_fenceModel, VGet(i * kFenceInterval + kXOffset + offsetX, offsetY, 110.0f));
                MV1DrawModel(m_fenceModel);
            }
        }



        // Armory看板描画（フェンスの中央に張り付ける）
        if (m_armoryBillboardModel.IsValid())
        {
            float billboardScale = 0.7f; // 0.5fから約1.4倍に拡大
            MV1SetScale(m_armoryBillboardModel, VGet(billboardScale, billboardScale, billboardScale));

            // Unity画像(Xが正面、Zが横幅)に基づき、正面をカメラ側(-Z方向)に向けるためY軸で90度回転させる
            // ユーザーの要望により、さらに180度回転させて裏側を向ける（90度 + 180度 = 270度 = -90度）
            MV1SetRotationXYZ(m_armoryBillboardModel, VGet(0.0f, -3.14159f * 0.5f, 0.0f));

            // 中央のフェンス(X=0付近)の高さ80くらい、少しだけ手前(Z=90.0)に配置
            // 揺れ演出のオフセット（XとY）を加算する
            VECTOR billboardPos = VGet(m_billboardShakeOffsetX, 80.0f + m_billboardShakeOffsetY, 90.0f);
            MV1SetPosition(m_armoryBillboardModel, billboardPos);
            MV1DrawModel(m_armoryBillboardModel);

            // 看板の文字部分を覆い隠すように、少しだけ手前(Zをマイナス方向)にTitleBanner画像を描画する
            if (m_banner.IsValid())
            {
                // Zバッファを有効にしたまま、3D空間内に2D画像（ビルボード）として描画
                // 看板より少し手前に配置（Z=88.0fなど）
                VECTOR bannerPos = VGet(billboardPos.x, billboardPos.y, billboardPos.z - 2.0f);
                
                // TitleBannerは1920x1080 (16:9)。元の幅160.0fから、看板の拡大に合わせて224.0f程度に拡大します。
                float bannerWidth = 224.0f;
                float bannerHeight = bannerWidth * (1080.0f / 1920.0f); 

                // カメラ固定を前提とし、看板の表面に平行な板ポリゴン（XY平面に平行）として画像を描画する
                VERTEX3D vertex[6];
                float halfW = bannerWidth * 0.5f;
                float halfH = bannerHeight * 0.5f;

                // 各頂点の座標を計算（Zは一定、看板より少し手前）
                VECTOR topLeft = VGet(bannerPos.x - halfW, bannerPos.y + halfH, bannerPos.z);
                VECTOR topRight = VGet(bannerPos.x + halfW, bannerPos.y + halfH, bannerPos.z);
                VECTOR bottomLeft = VGet(bannerPos.x - halfW, bannerPos.y - halfH, bannerPos.z);
                VECTOR bottomRight = VGet(bannerPos.x + halfW, bannerPos.y - halfH, bannerPos.z);

                // ポリゴン1 (左上、右上、左下)
                vertex[0].pos = topLeft; vertex[0].norm = VGet(0.0f, 0.0f, -1.0f);
                vertex[0].dif = GetColorU8(255, 255, 255, 255); vertex[0].spc = GetColorU8(0, 0, 0, 0);
                vertex[0].u = 0.0f; vertex[0].v = 0.0f; vertex[0].su = 0.0f; vertex[0].sv = 0.0f;

                vertex[1].pos = topRight; vertex[1].norm = VGet(0.0f, 0.0f, -1.0f);
                vertex[1].dif = GetColorU8(255, 255, 255, 255); vertex[1].spc = GetColorU8(0, 0, 0, 0);
                vertex[1].u = 1.0f; vertex[1].v = 0.0f; vertex[1].su = 0.0f; vertex[1].sv = 0.0f;

                vertex[2].pos = bottomLeft; vertex[2].norm = VGet(0.0f, 0.0f, -1.0f);
                vertex[2].dif = GetColorU8(255, 255, 255, 255); vertex[2].spc = GetColorU8(0, 0, 0, 0);
                vertex[2].u = 0.0f; vertex[2].v = 1.0f; vertex[2].su = 0.0f; vertex[2].sv = 0.0f;

                // ポリゴン2 (右上、右下、左下)
                vertex[3].pos = topRight; vertex[3].norm = VGet(0.0f, 0.0f, -1.0f);
                vertex[3].dif = GetColorU8(255, 255, 255, 255); vertex[3].spc = GetColorU8(0, 0, 0, 0);
                vertex[3].u = 1.0f; vertex[3].v = 0.0f; vertex[3].su = 0.0f; vertex[3].sv = 0.0f;

                vertex[4].pos = bottomRight; vertex[4].norm = VGet(0.0f, 0.0f, -1.0f);
                vertex[4].dif = GetColorU8(255, 255, 255, 255); vertex[4].spc = GetColorU8(0, 0, 0, 0);
                vertex[4].u = 1.0f; vertex[4].v = 1.0f; vertex[4].su = 0.0f; vertex[4].sv = 0.0f;

                vertex[5].pos = bottomLeft; vertex[5].norm = VGet(0.0f, 0.0f, -1.0f);
                vertex[5].dif = GetColorU8(255, 255, 255, 255); vertex[5].spc = GetColorU8(0, 0, 0, 0);
                vertex[5].u = 0.0f; vertex[5].v = 1.0f; vertex[5].su = 0.0f; vertex[5].sv = 0.0f;

                // 設定を一時的に変更
                SetLightEnable(false);
                SetUseLighting(false);
                SetUseBackCulling(false);

                // バナー画像の透明部分がZバッファを上書きして背景をくり抜くのを防ぐため、
                // バナー描画時のみZバッファへの書き込みを無効化する（テストは有効なままなので看板には埋もれない、あるいは両方無効化する）
                SetWriteZBuffer3D(false);

                // アルファブレンドを有効化
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

                // 透明度付きの板ポリゴンとして描画
                DrawPolygon3D(vertex, 2, m_banner.Get(), true);

                // 設定を元に戻す
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                SetWriteZBuffer3D(true);
                SetUseBackCulling(true);
                SetUseLighting(true);
                SetLightEnable(true);
            }
        }

        // ゾンビ描画
        if (m_zombieModel.IsValid())
        {
            for (const auto& zombie : m_zombies)
            {
                MV1SetPosition(m_zombieModel, zombie.pos);
                MV1SetRotationXYZ(m_zombieModel, VGet(0.0f, zombie.angleY, 0.0f));
                MV1SetScale(m_zombieModel, VGet(1.0f, 1.0f, 1.0f));

                // アニメーションをアタッチして時間を設定し、描画後にデタッチする
                int attachIndex = MV1AttachAnim(m_zombieModel, zombie.animIndex, -1, false);
                MV1SetAttachAnimTime(m_zombieModel, attachIndex, zombie.animTime);

                MV1DrawModel(m_zombieModel);

                MV1DetachAnim(m_zombieModel, attachIndex);
            }
        }

        // チュートリアル用背景モデルの描画（ゾンビよりも奥の遠景として配置）
        SetLightEnable(false);
        SetUseLighting(false);

        float bgScale = 1.0f;
        float baseZ = 2000.0f; // 奥に配置する基準Z座標
        float baseY = -40.0f;  // 床モデルと同じ高さに合わせるためのYオフセット

        // 画像内の中央に並ぶ2つの白い建物（HangarV3）
        // HangarV3.mv1 (幅1350, 高さ936 -> Yオフセット468)
        if (m_hangarV3Model.IsValid())
        {
            MV1SetScale(m_hangarV3Model, VGet(bgScale, bgScale, bgScale));
            MV1SetRotationXYZ(m_hangarV3Model, VGet(0.0f, 0.0f, 0.0f)); // 正面を向ける
            
            // 左側の白い建物 (幅の半分675をX軸のマイナス方向にずらすと、右端が原点に合う)
            MV1SetPosition(m_hangarV3Model, VGet(-675.0f, baseY + 468.0f, baseZ));
            MV1DrawModel(m_hangarV3Model);

            // 右側の白い建物 (左端が原点に合う)
            MV1SetPosition(m_hangarV3Model, VGet(675.0f, baseY + 468.0f, baseZ));
            MV1DrawModel(m_hangarV3Model);
        }
        
        // 画像内の左端にあるオレンジの建物（Hangar）
        // Hangar.mv1 (幅720, 高さ502 -> Yオフセット251, 奥行967)
        if (m_hangarModel.IsValid())
        {
            MV1SetScale(m_hangarModel, VGet(bgScale, bgScale, bgScale));
            // Unity画像では側面（奥行がある方）が見えているため90度回転させる
            MV1SetRotationXYZ(m_hangarModel, VGet(0.0f, 3.14159f * 0.5f, 0.0f));
            
            // 左側の白い建物の左端（X = -1350）のさらに左に配置する
            // 90度回転させたためX方向の幅は967になる。その半分(483.5)を足す。
            MV1SetPosition(m_hangarModel, VGet(-1350.0f - 483.5f, baseY + 251.0f, baseZ));
            MV1DrawModel(m_hangarModel);
        }
        
        // 画像内の右端に積み上げられているコンテナ群
        // Container.mv1 (幅800, 高さ300 -> Yオフセット150)
        if (m_containerModel.IsValid())
        {
            MV1SetScale(m_containerModel, VGet(bgScale, bgScale, bgScale));
            MV1SetRotationXYZ(m_containerModel, VGet(0.0f, 0.0f, 0.0f)); // 長い側面を向ける
            
            // 右側の白い建物の右端（X = 1350）のさらに右に配置する
            for (int x = 0; x < 3; ++x)
            {
                // コンテナの幅800ずつ右へずらす
                float posX = 1350.0f + 400.0f + (x * 800.0f);
                
                // 3段積み上げる（高さ300ずつ上へ）
                for (int y = 0; y < 3; ++y)
                {
                    float posY = baseY + 150.0f + (y * 300.0f);
                    MV1SetPosition(m_containerModel, VGet(posX, posY, baseZ));
                    MV1DrawModel(m_containerModel);
                }
            }
        }

        SetLightEnable(true);
        SetUseLighting(true);
        



        // Zバッファの影響を受けないようにする（2D描画用）
        SetUseZBuffer3D(false);
        SetWriteZBuffer3D(false);

        const char* gameStartText = "Press Left Click to Start Game";
        int textWidth = GetDrawStringWidthToHandle(gameStartText, -1, m_font);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_gameStartTextAlpha);
        DrawFormatStringToHandle((Game::GetScreenWidth() - textWidth) * 0.5f, Game::GetScreenHeight() - 180, 0xffffff, m_font, gameStartText);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

