#include "StageObject.h"
#include "EffekseerForDXLib.h"
#include <cassert>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void StageObject::Init(int duplicateHandle, Vec3 pos, Vec3 rot, Vec3 scale)
{
	m_modelHandle = duplicateHandle;
	assert(m_modelHandle >= 0);
	m_pos = pos;
	m_rot = rot;
	m_scale = scale;

	// Unity側で既に座標系変換が行われているため、位置はそのまま使用
	// Unity側: pos = new Vector3(pos.x, pos.y, -pos.z) でZ軸が反転済み
	// DxLib側では、Unity側で既に変換済みの値を使用するため、そのまま適用
	VECTOR dxPos = pos.ToDxVECTOR();
	MV1SetPosition(m_modelHandle, dxPos); // 位置設定

	// 回転をラジアンに変換
	// Unity側で既に座標系変換が行われている（rot = new Vector3(rot.x, -rot.y + config.rotationOffset, rot.z)）
	// CSVファイルには既に変換済みの値が入っているため、そのまま使用
	// ただし、UnityとDxLibで座標系が異なるため、Y軸回転を再度反転する必要がある
	float rotXRad = rot.x * (M_PI / 180.0f);
	float rotYRad = rot.y * (M_PI / 180.0f); // CSVには既に反転済みの値が入っているが、DxLib側で正しい向きにするために再度反転
	float rotZRad = rot.z * (M_PI / 180.0f);

	// DxLibの座標系に合わせて回転を適用
	// Unity側で既にY軸回転が反転されているため、DxLib側で再度反転して正しい向きにする
	MV1SetRotationXYZ(m_modelHandle, VGet(rotXRad, -rotYRad, rotZRad)); // Y軸回転を再度反転
	MV1SetScale(m_modelHandle, VGet(scale.x * 0.01f, scale.y * 0.01f, scale.z * 0.01f)); // スケール設定
}

void StageObject::Draw()
{
	MV1DrawModel(m_modelHandle);
}