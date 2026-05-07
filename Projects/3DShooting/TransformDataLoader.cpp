#include "DxLib.h"
#include "TransformDataLoader.h"
#include <fstream>
#include <sstream>

namespace
{
	// 要素数 (Name, Pos*3, Rot*3, Scale*3, Attack, HP, ChaseSpeed, WalkSpeed, RunSpeed, ARInit, SGInit, ARPower, SGPower, TackleCD, TackleSpd, TackleDmg, MaxShield, ShieldRegen)
	constexpr int kElementNum = 24; 

	// Unityの座標系からDxLibの座標系への変換係数（位置情報には適用しない）
	constexpr float kUnityToDxPos = 100.0f; 
}

std::vector<ObjectTransformData> TransformDataLoader::LoadDataCSV(const char* fileName)
{
	// データを格納する配列
	std::vector<ObjectTransformData> Object;

	// ファイルを開く
	std::ifstream file(fileName);

	// もしもファイルを開けなかったら
	if (!file.is_open())
	{
		return Object; // 空の配列を返す
	}

	// 位置情報に100倍のスケールを適用する（Unity側の座標系からDxLibの座標系への変換のため）
	std::string fileNameStr(fileName);
	bool applyPositionScale = true; // すべてのCSVファイルで位置情報に100倍のスケールを適用

	// 1行ずつ読み込む
	std::string line;
	//最初のヘッダーはスキップ
	bool isHeader = true;

	// CSVファイルの各行を読み込む
	while (std::getline(file, line))
	{
		if (isHeader)
		{
			isHeader = false; // ヘッダー行をスキップ
			continue;
		}

		std::stringstream ss(line);
		ObjectTransformData data;

		// カンマで分割して各要素を読み込む
		std::string element;
		int index = 0;

		while (std::getline(ss, element, ',') && index < kElementNum)
		{
			// 空のセルをチェック
			if (element.empty())
			{
				index++;
				continue;
			}

			switch (index)
			{
			case 0: // 名前
				data.name = element;
				break;
			case 1: // 位置X
				try { data.pos.x = applyPositionScale ? (std::stof(element) * kUnityToDxPos) : std::stof(element); }
				catch (...) { data.pos.x = 0.0f; }
				break;
			case 2: // 位置Y
				try { data.pos.y = applyPositionScale ? (std::stof(element) * kUnityToDxPos) : std::stof(element); }
				catch (...) { data.pos.y = 0.0f; }
				break;
			case 3: // 位置Z
				try { data.pos.z = applyPositionScale ? (std::stof(element) * kUnityToDxPos) : std::stof(element); }
				catch (...) { data.pos.z = 0.0f; }
				break;
			case 4: // 回転X
				try { data.rot.x = std::stof(element); }
				catch (...) { data.rot.x = 0.0f; }
				break;
			case 5: // 回転Y
				try { data.rot.y = std::stof(element); }
				catch (...) { data.rot.y = 0.0f; }
				break;
			case 6: // 回転Z
				try { data.rot.z = std::stof(element); }
				catch (...) { data.rot.z = 0.0f; }
				break;
			case 7: // スケールX
				try { data.scale.x = std::stof(element); }
				catch (...) { data.scale.x = 1.0f; }
				break;
			case 8: // スケールY
				try { data.scale.y = std::stof(element); }
				catch (...) { data.scale.y = 1.0f; }
				break;
			case 9: // スケールZ
				try { data.scale.z = std::stof(element); }
				catch (...) { data.scale.z = 1.0f; }
				break;
			case 10: // 攻撃力
				try { data.attack = std::stof(element); }
				catch (...) { data.attack = 0.0f; }
				break;
			case 11: // 体力
				try { data.hp = std::stof(element); }
				catch (...) { data.hp = 0.0f; }
				break;
			case 12: // ChaseSpeed (敵用)
				try { data.chaseSpeed = std::stof(element); }
				catch (...) { data.chaseSpeed = 0.0f; }
				break;
			case 13: // WalkSpeed (プレイヤー用)
				try { data.speed = std::stof(element); }
				catch (...) { data.speed = 0.0f; }
				break;
			case 14: // RunSpeed (プレイヤー用)
				try { data.runSpeed = std::stof(element); }
				catch (...) { data.runSpeed = 0.0f; }
				break;
			case 15: // ARInitialAmmo
				try { data.arInitAmmo = std::stoi(element); }
				catch (...) { data.arInitAmmo = 0; }
				break;
			case 16: // SGInitialAmmo
				try { data.sgInitAmmo = std::stoi(element); }
				catch (...) { data.sgInitAmmo = 0; }
				break;
			case 17: // ARBulletPower
				try { data.bulletPower = std::stof(element); }
				catch (...) { data.bulletPower = 0.0f; }
				break;
			case 18: // SGBulletPower
				try { data.sgBulletPower = std::stof(element); }
				catch (...) { data.sgBulletPower = 0.0f; }
				break;
			case 19: // TackleCooldown
				try { data.tackleCooldown = std::stof(element); }
				catch (...) { data.tackleCooldown = 0.0f; }
				break;
			case 20: // TackleSpeed
				try { data.tackleSpeed = std::stof(element); }
				catch (...) { data.tackleSpeed = 0.0f; }
				break;
			case 21: // TackleDamage
				try { data.tackleDamage = std::stof(element); }
				catch (...) { data.tackleDamage = 0.0f; }
				break;
			case 22: // MaxShieldDurability
				try { data.maxShieldDurability = std::stof(element); }
				catch (...) { data.maxShieldDurability = 0.0f; }
				break;
			case 23: // ShieldRegenRate
				try { data.shieldRegenRate = std::stof(element); }
				catch (...) { data.shieldRegenRate = 0.0f; }
				break;
			default:
				break;
			}
			index++;
		}
		Object.push_back(data); // 読み込んだデータを配列に追加
	}

	// ファイルを閉じる
	file.close();
	return Object; // 読み込んだデータの配列を返す
}