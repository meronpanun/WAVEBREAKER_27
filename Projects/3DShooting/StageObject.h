#include "Vec3.h"
#include <string>

/// <summary>
/// ステージオブジェクトクラス
/// </summary>
class StageObject
{
public:
	void Init(int duplicateHandle, Vec3 pos, Vec3 rot, Vec3 scale);
	void Draw();

	Vec3 GetPos() const { return m_pos; }
	Vec3 GetScale() const { return m_scale; }

private:
	int m_modelHandle;
	Vec3 m_pos;
	Vec3 m_rot;
	Vec3 m_scale;
};