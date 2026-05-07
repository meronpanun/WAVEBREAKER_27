#pragma once
#include "DxLib.h"

// スポーンエリア情報 (SpawnAreaData.csv)
struct SpawnAreaInfo
{
  int type = 0;        // タイプ (0:Main, 1:Tutorial)
  VECTOR center = {0}; // 中心座標
  VECTOR size = {0};   // サイズ (Scale)
};
