#pragma once

// 攻撃の種類を定義するenum
enum class AttackType
{
  None,
  Shoot,       // 射撃
  Shotgun,     // ショットガン
  Tackle,      // タックル
  ShieldThrow, // 盾投げ
  Parry        // パリィ
};