#pragma once

#include "pch.h"
#include "Vector3.h"

inline float GetRandomFloat(float min, float max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

inline int GetRandomInt(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

inline int ManhattanDistance(const Vector3& a, const Vector3& b)
{
    return static_cast<int>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

enum class Direction : uint8_t
{
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3
};

struct AttackRect
{
    Vector3 topLeft;
    Vector3 bottomRight;

    bool Contains(const Vector3& pos) const
    {
        return (pos.x >= topLeft.x && pos.x <= bottomRight.x &&
            pos.y >= topLeft.y && pos.y <= bottomRight.y);
    }
};

inline AttackRect GetAttackRect(const Vector3& playerPos, Direction dir)
{
    const float offset = 20.0f;
    const float length1 = 10.0f;
    const float length2 = 30.0f;

    AttackRect rect;

    switch (dir)
    {
    case Direction::Left:
        rect.topLeft.x = playerPos.x - offset - length1;
        rect.bottomRight.x = playerPos.x;
        rect.topLeft.y = playerPos.y - length2 / 2;
        rect.bottomRight.y = playerPos.y + length2 / 2;
        break;

    case Direction::Right:
        rect.topLeft.x = playerPos.x;
        rect.bottomRight.x = playerPos.x + offset + length1;
        rect.topLeft.y = playerPos.y - length2 / 2;
        rect.bottomRight.y = playerPos.y + length2 / 2;
        break;

    case Direction::Up:
        rect.topLeft.y = playerPos.y - offset - length1;
        rect.bottomRight.y = playerPos.y;
        rect.topLeft.x = playerPos.x - length2 / 2;
        rect.bottomRight.x = playerPos.x + length2 / 2;
        break;

    case Direction::Down:
        rect.topLeft.y = playerPos.y;
        rect.bottomRight.y = playerPos.y + offset + length1;
        rect.topLeft.x = playerPos.x - length2 / 2;
        rect.bottomRight.x = playerPos.x + length2 / 2;
        break;
    }

    return rect;
}

inline std::string Sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);

    char output[65]; // 64자리 해시 + 널 종료
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf_s(output + (i * 2), 3, "%02x", hash[i]);

    output[64] = 0;
    return std::string(output);
}

// SQL 인젝션 방지
inline std::string EscapeSQL(const std::string& input) 
{
    std::string escaped;
    for (char c : input) 
    {
        if (c == '\'') escaped += "''"; // 작은 따옴표 2개로 변환
        else escaped += c;
    }
    return escaped;
}