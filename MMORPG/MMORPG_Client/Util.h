#pragma once

#include "pch.h"

enum class Direction : uint8_t
{
    Left = 0,
    Right = 1,
    Up = 2,
    Down = 3
};

struct AttackInfo
{
    DWORD startTime;
    CPoint startPos;
    Direction dir;
};