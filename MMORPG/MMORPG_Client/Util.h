#pragma once

#include "pch.h"
#include "Vector3.h"

enum class Direction : uint8_t
{
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3
};

struct AttackInfo
{
    DWORD startTime;
    CPoint startPos;
    Direction dir;
};

struct PlayerInput 
{
    uint32_t frameID;
    Direction dir;
};

inline Vector3 Lerp(const Vector3& from, const Vector3& to, float t)
{
	return Vector3(
		from.x + (to.x - from.x) * t,
		from.y + (to.y - from.y) * t,
		from.z + (to.z - from.z) * t
	);
}