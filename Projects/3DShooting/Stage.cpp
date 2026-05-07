#include "Stage.h"
#include "TransformDataLoader.h"
#include "DxLib.h"
#include <unordered_map>
#include <cstdio>
#include <fstream>
#include <sstream>

bool Stage::s_shouldDrawCollision = false;
bool Stage::s_shouldDrawTutorialCollision = false;

void Stage::LoadStage(bool isTutorial)
{
	Clear();
	m_isTutorial = isTutorial;

	TransformDataLoader loader;
	std::vector<ObjectTransformData> objectDataList;

	if (isTutorial)
	{
		// 旧Stage.csvの読み込み処理 (チュートリアル用)
		objectDataList = loader.LoadDataCSV("Data/CSV/TutorialStageTransformData.csv");

		// ステージ当たり判定データの読み込み (チュートリアル用)
		LoadCollisionData("Data/CSV/TutorialStageCollisionData.csv");
	}
	else
	{
		// MainStageTransformData.csvの読み込み (メインステージ用)
		objectDataList = loader.LoadDataCSV("Data/CSV/MainStageTransformData.csv");

		// ステージ当たり判定データの読み込み (メインステージ用)
		LoadCollisionData("Data/CSV/MainStageCollisionData.csv");
	}

	int loadedCount = 0; // 読み込んだオブジェクト数
	int skippedCount = 0; // スキップされたオブジェクト数

	for (const auto& data : objectDataList)
	{
		std::string modelPath;

		if (isTutorial)
		{
			// 旧モデルの処理
			if (data.name == "UNIConcrete")
			{
				modelPath = "Data/Model/UNIConcrete.mv1";
			}
			else if (data.name == "RoadFloor")
			{
				modelPath = "Data/Model/RoadFloor.mv1";
			}
			else if (data.name == "HangarV3")
			{
				modelPath = "Data/Model/HangarV3.mv1";
			}	
			else if (data.name == "HangarV1")
			{
				modelPath = "Data/Model/Hangar.mv1";
			}	
			else if (data.name == "CargoContainer")
			{
				modelPath = "Data/Model/Container.mv1";
			}
		}
		else
		{
			// 新モデルの処理
			if (data.name == "BarrierGroup1A")
			{
				modelPath = "Data/Model/BarrierGroup1A.mv1";
			}
			else if (data.name == "BasicStairs1B")
			{
				modelPath = "Data/Model/BasicStairs1B.mv1";
			}
			else if (data.name == "BlockPlatform1B")
			{
				modelPath = "Data/Model/BlockPlatform1B.mv1";
			}
			else if (data.name == "Block_Platform_Corner_1B(Mirrored)")
			{
				modelPath = "Data/Model/Block_Platform_Corner_1B(Mirrored).mv1";
			}
			else if (data.name == "Block_Platform_Corner_1B")
			{
				modelPath = "Data/Model/Block_Platform_Corner_1B.mv1";
			}
			else if (data.name == "Chain")
			{
				modelPath = "Data/Model/Chain.mv1";
			}
			else if (data.name == "FloorA")
			{
				modelPath = "Data/Model/FloorA.mv1";
			}
			else if (data.name == "FloorB")
			{
				modelPath = "Data/Model/FloorB.mv1";
			}
			else if (data.name == "Rock3")
			{
				modelPath = "Data/Model/Rock3.mv1";
			}
			else if (data.name == "Rock6")
			{
				modelPath = "Data/Model/Rock6.mv1";
			}
		}

		if (modelPath.empty())
		{
			skippedCount++;
			continue;
		}

		// モデルキャッシュの確認
		int originalHandle = -1;
		if (m_modelCache.find(modelPath) != m_modelCache.end())
		{
			originalHandle = m_modelCache[modelPath];
		}
		else
		{
			// 新規読み込み
			originalHandle = MV1LoadModel(modelPath.c_str());
			if (originalHandle != -1)
			{
				m_modelCache[modelPath] = originalHandle;
			}
		}

		if (originalHandle != -1)
		{
			// モデルの複製
			int duplicateHandle = MV1DuplicateModel(originalHandle);
			if (duplicateHandle != -1)
			{
				StageObject obj;
				Vec3 pos = { data.pos.x, data.pos.y, data.pos.z };
				Vec3 rot = { data.rot.x, data.rot.y, data.rot.z };
				Vec3 scale = { data.scale.x, data.scale.y, data.scale.z };
				obj.Init(duplicateHandle, pos, rot, scale);
				m_objects.emplace_back(std::move(obj));
				loadedCount++;
			}
		}
	}

	CalculateBounds();
}

void Stage::Clear()
{
	m_objects.clear();
	m_collisionData.clear();
}

Stage::~Stage()
{
	// キャッシュされたモデルの解放
	for (auto& pair : m_modelCache)
	{
		MV1DeleteModel(pair.second);
	}
	m_modelCache.clear();
}

void Stage::Draw()
{
	for (auto& obj : m_objects)
	{
		obj.Draw();
	}

	// 当たり判定のデバッグ描画
	bool isDraw = m_isTutorial ? s_shouldDrawTutorialCollision : s_shouldDrawCollision;
	if (isDraw)
	{
		for (const auto& col : m_collisionData)
		{
			DrawTriangle3D(col.v1, col.v2, col.v3, 0xff0000, false);
		}
	}
}

void Stage::LoadCollisionData(const char* fileName)
{
	std::ifstream file(fileName);
	if (!file.is_open()) return;

	std::string line;
	bool isHeader = true;

	while (std::getline(file, line))
	{
		if (isHeader)
		{
			isHeader = false;
			continue;
		}

		std::stringstream ss(line);
		std::string element;
		StageCollisionData data;
		int index = 0;

		while (std::getline(ss, element, ','))
		{
			if (element.empty())
			{
				index++;
				continue;
			}

			try
			{
				switch (index)
				{
				case 0: data.name = element; break;
				case 1: data.v1.x = std::stof(element); break;
				case 2: data.v1.y = std::stof(element); break;
				case 3: data.v1.z = std::stof(element); break;
				case 4: data.v2.x = std::stof(element); break;
				case 5: data.v2.y = std::stof(element); break;
				case 6: data.v2.z = std::stof(element); break;
				case 7: data.v3.x = std::stof(element); break;
				case 8: data.v3.y = std::stof(element); break;
				case 9: data.v3.z = std::stof(element); break;
				}
			}
			catch (...)
			{
				// エラー処理（必要に応じてログ出力など）
			}
			index++;
		}
		m_collisionData.push_back(data);
	}
}

void Stage::CalculateBounds()
{
	if (m_objects.empty())
	{
		m_minBounds = VGet(-1000, 0, -1000);
		m_maxBounds = VGet(1000, 0, 1000);
		return;
	}

	m_minBounds = VGet(FLT_MAX, FLT_MAX, FLT_MAX);
	m_maxBounds = VGet(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const auto& obj : m_objects)
	{
		Vec3 pos = obj.GetPos();
		// スケールも考慮する必要があるが、簡易的に位置だけで計算するか、
		// モデルのバウンディングボックスを取得するのが理想的。
		// ここでは、各オブジェクトがそれなりの大きさを持つと仮定して少し余裕を持たせる
		float margin = 200.0f; // マージンを少し抑える

		if (pos.x - margin < m_minBounds.x) m_minBounds.x = pos.x - margin;
		if (pos.y - margin < m_minBounds.y) m_minBounds.y = pos.y - margin;
		if (pos.z - margin < m_minBounds.z) m_minBounds.z = pos.z - margin;
		if (pos.x + margin > m_maxBounds.x) m_maxBounds.x = pos.x + margin;
		if (pos.y + margin > m_maxBounds.y) m_maxBounds.y = pos.y + margin;
		if (pos.z + margin > m_maxBounds.z) m_maxBounds.z = pos.z + margin;
	}
	
	// Y座標のマージン調整
	m_minBounds.y -= 100.0f;
	m_maxBounds.y += 500.0f;
}
