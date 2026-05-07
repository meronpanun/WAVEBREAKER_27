#pragma once

/// <summary>
/// シーン基底クラス
/// </summary>
class SceneBase abstract
{
public:
  SceneBase() = default;
  virtual ~SceneBase() = default;

  virtual void Init() abstract;
  virtual SceneBase *Update() abstract;
  virtual void Draw() abstract;

  /// <summary>
  /// ロード中かどうかを取得する
  /// </summary>
  /// <returns>ロード中ならtrue</returns>
  virtual bool IsLoading() const { return false; }
};
