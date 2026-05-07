#pragma once
#include "StageObject.h"
#include <vector>
#include <memory>

/// <summary>
/// ステージクラス
/// </summary>
class Stage
{
public:
	struct StageCollisionData
	{
		std::string name;
		VECTOR v1;
		VECTOR v2;
		VECTOR v3;
	};

	~Stage();

	void LoadStage(bool isTutorial);
	void Clear();
	void Draw();

	const std::vector<StageCollisionData>& GetCollisionData() const { return m_collisionData; }

	VECTOR GetMinBounds() const { return m_minBounds; }
	VECTOR GetMaxBounds() const { return m_maxBounds; }

	// デバッグ用
	static void SetDrawCollision(bool isDraw) { s_shouldDrawCollision = isDraw; }
	static bool ShouldDrawCollision() { return s_shouldDrawCollision; }
	static void SetDrawTutorialCollision(bool isDraw) { s_shouldDrawTutorialCollision = isDraw; }
	static bool ShouldDrawTutorialCollision() { return s_shouldDrawTutorialCollision; }

private:
	std::vector<StageObject> m_objects;
	std::vector<StageCollisionData> m_collisionData;
	std::unordered_map<std::string, int> m_modelCache;

	static bool s_shouldDrawCollision;
	static bool s_shouldDrawTutorialCollision;
	bool m_isTutorial;

	void LoadCollisionData(const char* fileName);
	void CalculateBounds();

	VECTOR m_minBounds;
	VECTOR m_maxBounds;
};

