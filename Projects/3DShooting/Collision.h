#pragma once
#include "EffekseerForDXLib.h"
#include "Stage.h"
#include <vector>

struct CollisionResult
{
    bool isGrounded = false;
    VECTOR groundNormal = VGet(0, 1, 0); // 初期値は真上
    std::string groundedObjectName;
};

class Collision
{
public:
    static CollisionResult CheckStageCollision(VECTOR& position, float capsuleHeight, float capsuleRadius, float colliderYOffset, const std::vector<Stage::StageCollisionData>& collisionData, const class CollisionGrid* pGrid = nullptr);

    static bool IntersectRayTriangle(const VECTOR& rayOrig, const VECTOR& rayDir, const VECTOR& v0, const VECTOR& v1, const VECTOR& v2, float& outT);
};
