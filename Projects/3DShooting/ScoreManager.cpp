#include "ScoreManager.h"
#include <cmath>
#include "TaskTutorialManager.h"
#include <fstream>
#include <algorithm>

namespace
{
    constexpr int   kBaseHeadShotScore  = 200;  // ヘッドショットの基本スコア
    constexpr int   kBaseBodyShotScore  = 100;  // ボディショットの基本スコア
    constexpr float kInitialComboRate   = 1.1f; // コンボ倍率の初期値
    constexpr int   kMaxHighScores      = 10;
    const char* kScoreFileName = "highscores.txt"; // スコア保存ファイル名
    constexpr int   kMinCountUpSpeed    = 50;   // カウントアップの最低速度(フレーム毎)
    constexpr float kCountUpRatio       = 0.05f;// 残り差分に対するカウントアップ割合
}

ScoreManager& ScoreManager::Instance()
{
    static ScoreManager instance;
    return instance;
}

ScoreManager::ScoreManager()
    : m_score(0)
    , m_combo(0)
    , m_maxCombo(0)
    , m_comboTimer(kComboGraceFrame)
    , m_totalScore(0)
    , m_bodyKillCount(0)
    , m_headKillCount(0)
    , m_lastComboRate(1.0f)
    , m_displayScore(0)
    , m_targetDisplayScore(0)
    , m_displayTotalScore(0)
    , m_targetTotalScore(0)
    , m_targetBodyKillCount(0)
    , m_targetHeadKillCount(0)
    , m_scoreCountUpSpeed(30)
{
    LoadScores(); // 初期化時にスコアを読み込み
}

// スコアを追加する
int ScoreManager::AddScore(bool isHeadShot)
{
    int baseScore = isHeadShot ? kBaseHeadShotScore : kBaseBodyShotScore;
    m_combo++;
    if (m_combo > m_maxCombo) m_maxCombo = m_combo; // 最大コンボ更新

    float comboRate = std::pow(kInitialComboRate, m_combo - 1);
    m_lastComboRate = comboRate;
    int add = static_cast<int>(baseScore * comboRate);
    m_score += add;
    m_totalScore += add; // 累計スコアにも加算
    m_comboTimer = kComboGraceFrame; // コンボ猶予リセット

    if (TaskTutorialManager::GetInstance() && TaskTutorialManager::GetInstance()->IsCompleted())
    {
        if (isHeadShot) 
        {
            m_headKillCount++;
        }
        else 
        {
            m_bodyKillCount++;
        }
    }
    return add;
}

void ScoreManager::Update()
{
    if (m_combo > 0 && m_comboTimer > 0) 
    {
        m_comboTimer--;
        if (m_comboTimer <= 0) 
        {
            m_combo = 0;
            m_lastComboRate = 1.0f;
            m_score = 0; // コンボが切れたらスコアもリセット
        }
    }

    // スコアカウントアップ演出
    // 差分の一定割合（最低kMinCountUpSpeed）ずつ増やすことで、
    // スコアが大きくても短時間でアニメーションが完了する
    if (m_displayScore < m_targetDisplayScore)
    {
        int diff = m_targetDisplayScore - m_displayScore;
        int add = (std::max)(kMinCountUpSpeed, static_cast<int>(diff * kCountUpRatio));
        add = (std::min)(add, diff); // 目標値を超えないよう上限設定
        m_displayScore += add;
    }
    if (m_displayTotalScore < m_targetTotalScore)
    {
        int diff = m_targetTotalScore - m_displayTotalScore;
        int add = (std::max)(kMinCountUpSpeed, static_cast<int>(diff * kCountUpRatio));
        add = (std::min)(add, diff); // 目標値を超えないよう上限設定
        m_displayTotalScore += add;
    }
}

// コンボをリセット
void ScoreManager::ResetCombo() 
{
    m_combo = 0;
}

// 現在のスコアを取得
int ScoreManager::GetScore() const 
{
    return m_score;
}

// ゲーム全体の累計スコアを取得
int ScoreManager::GetCombo() const 
{
    return m_combo;
}

// スコアを保存する
void ScoreManager::SaveScore(int score)
{
    // スコアをリストに追加
    m_highScores.push_back(score);
    // 降順にソート
    std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());

    // 最大数を超えた場合は削除
    if (m_highScores.size() > kMaxHighScores)
    {
        m_highScores.resize(kMaxHighScores);
    }
        
    // ファイルに保存
    std::ofstream file(kScoreFileName);
    if (file.is_open())
    {
        for (int s : m_highScores)
        {
            file << s << std::endl;
        }
        file.close();
    }
}

// スコアを読み込む
void ScoreManager::LoadScores()
{
    m_highScores.clear();
    std::ifstream file(kScoreFileName);
    if (file.is_open())
    {
        int score;
        while (file >> score)
        {
            m_highScores.push_back(score);
        }
        file.close();
        // 降順にソート
        std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
    }
}

// 表示用の値をリセット
void ScoreManager::ResetDisplayValues() 
{
    m_displayScore      = 0;
    m_displayTotalScore = 0;
}

// 表示用の目標値を設定
void ScoreManager::SetTargetDisplayValues(int score, int totalScore, int bodyKill, int headKill) 
{
    m_targetDisplayScore  = score;
    m_targetTotalScore    = totalScore;
    m_targetBodyKillCount = bodyKill;
    m_targetHeadKillCount = headKill;
}

void ScoreManager::ResetAll()
{
    m_score = 0;
    m_totalScore = 0;
    m_combo = 0;
    m_maxCombo = 0;
    m_comboTimer = 0;
    m_lastComboRate = 1.0f;
    m_bodyKillCount = 0;
    m_headKillCount = 0;
    m_displayScore = 0;
    m_targetDisplayScore = 0;
    m_displayTotalScore = 0;
    m_targetTotalScore = 0;
    m_targetBodyKillCount = 0;
    m_targetHeadKillCount = 0;
}

