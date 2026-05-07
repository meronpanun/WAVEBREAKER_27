#pragma once
#include <vector>

/// <summary>
/// スコア管理クラス
/// </summary>
class ScoreManager
{
public:
    ScoreManager();

    void Update();

    /// <summary>
    /// スコアマネージャーのインスタンスを取得
    /// </summary>
    /// <returns>スコアマネージャーのインスタンス</returns>
    static ScoreManager& Instance();

    /// <summary>
    /// スコアを加算する
    /// </summary>
    /// <param name="isHeadShot">ヘッドショットならtrue</param>
    /// <returns>加算されたスコア</returns>
    int AddScore(bool isHeadShot);

    /// <summary>
    /// コンボをリセットする
    /// </summary>
    void ResetCombo();

    /// <summary>
    /// 現在のスコアを取得
    /// </summary>
    /// <returns>現在のスコア</returns>
    int GetScore() const;

    /// <summary>
    /// ゲーム全体の累計スコアを取得
    /// </summary>
    /// <returns>累計スコア</returns>
    int GetTotalScore() const { return m_totalScore; }

    /// <summary>
    /// 現在のコンボ数を取得
    /// </summary>
    /// <returns>現在のコンボ数</returns>
    int GetCombo() const;

    /// <summary>
    /// ボディショットキル数を取得
    /// </summary>
    /// <returns>ボディショットキル数</returns>
    int GetBodyKillCount() const { return m_bodyKillCount; }

    /// <summary>
    /// ヘッドショットキル数を取得
    /// </summary>
    /// <returns>ヘッドショットキル数</returns>
    int GetHeadKillCount() const { return m_headKillCount; }
    
    /// <summary>
    /// 総撃破数を取得(ボディ+ヘッド)
    /// </summary>
    int GetTotalDefeatedCount() const { return m_bodyKillCount + m_headKillCount; }

    /// <summary>
    /// 最後のコンボ倍率を取得
    /// </summary>
    /// <returns>最後のコンボ倍率</returns>
    float GetLastComboRate() const { return m_lastComboRate; }

    /// <summary>
    /// スコア保存・読み込み機能 
    /// </summary>
    /// <param name="score">保存するスコア</param>
    void SaveScore(int score);

    /// <summary>
    /// スコアを読み込む
    /// </summary>
    void LoadScores();

    /// <summary>
    /// ハイスコアのリストを取得
    /// </summary>
    /// <returns>ハイスコアのリスト</returns>
    const std::vector<int>& GetHighScores() const { return m_highScores; }

    /// <summary>
    /// 最高スコアを取得
    /// </summary>
    /// <returns>最高スコア</returns>
    int GetHighestScore() const { return m_highScores.empty() ? 0 : m_highScores[0]; }

    /// <summary>
	/// 表示用スコアを取得
    /// </summary>
	/// <returns>表示用スコア</returns>
    int GetDisplayScore() const { return m_displayScore; }

    /// <summary>
	/// 表示用スコアのリセット
    /// </summary>
    void ResetDisplayScore() { m_displayScore = 0; m_targetDisplayScore = m_score; }

    /// <summary>
	/// 表示用スコアの目標値を設定
    /// </summary>
	/// <param name="score">目標スコア</param>
    void SetTargetDisplayScore(int score) { m_targetDisplayScore = score; }

    /// <summary>
	/// カウントアップ速度を設定
    /// </summary>
	/// <param name="speed">速度</param>
    void SetScoreCountUpSpeed(int speed) { m_scoreCountUpSpeed = speed; }

    /// <summary>
	/// 表示用累計スコアを取得
    /// </summary>
	/// <returns>表示用累計スコア</returns>
    int GetDisplayTotalScore() const { return m_displayTotalScore; }

    /// <summary>
	/// 表示用累計スコアのリセット
    /// </summary>
    void ResetDisplayValues();

    /// <summary>
	/// 表示用の目標値を設定
    /// </summary>
	/// <param name="score">目標スコア</param>
	/// <param name="totalScore">目標累計スコア</param>
	/// <param name="bodyKill">目標ボディショットキル数</param>
	/// <param name="headKill">目標ヘッドショットキル数</param>
    void SetTargetDisplayValues(int score, int totalScore, int bodyKill, int headKill);

    /// <summary>
    /// 最大コンボ数を取得
    /// </summary>
    /// <returns>最大コンボ数</returns>
    int GetMaxCombo() const { return m_maxCombo; }

    void ResetAll(); // スコア・キル数・コンボなど全リセット
    
    // 猶予時間定数
    static constexpr int kComboGraceFrame = 240; // 4.0秒 (90 -> 240)

    /// <summary>
    /// コンボタイマーの現在値を取得
    /// </summary>
    int GetComboTimer() const { return m_comboTimer; }

private:
	std::vector<int> m_highScores; // ハイスコアリスト

	// スコア関連
    int m_score;               // 現在のスコア
    int m_totalScore;          // ゲーム全体の累計スコア

	// コンボ関連
    int m_combo;               // 現在のコンボ数
    int m_maxCombo;            // 最大コンボ数
    int m_comboTimer;          // コンボ継続猶予タイマー
    float m_lastComboRate;     // 最後のコンボ倍率

	// キル数関連
    int m_bodyKillCount;       // ボディショットキル数
    int m_headKillCount;       // ヘッドショットキル数

    // カウントアップ演出用
	int m_displayScore;        // 表示用スコア
    int m_targetDisplayScore;  // 表示用スコアの目標値
    int m_displayTotalScore;   // 表示用累計スコア
    int m_targetTotalScore;    // 表示用累計スコアの目標値
    int m_targetBodyKillCount; // 表示用ボディショットキル数の目標値
	int m_targetHeadKillCount; // 表示用ヘッドショットキル数の目標値
    int m_scoreCountUpSpeed;   // カウントアップ速度
};

