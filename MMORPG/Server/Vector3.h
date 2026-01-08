#pragma once

struct Vector3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	Vector3() = default;

	Vector3(float x, float y, float z)
		: x(x), y(y), z(z) {}

	// 덧셈
	Vector3 operator+(const Vector3& other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	// 뺄셈
	Vector3 operator-(const Vector3& other) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	// 스칼라 곱
	Vector3 operator*(float scalar) const
	{
		return { x * scalar, y * scalar, z * scalar };
	}

	// 벡터 정규화
	Vector3 Normalize() const
	{
		float length = sqrtf(x * x + y * y); // z 무시
		if (length == 0.0f)
			return { 0.0f, 0.0f, 0.0f };
		return { x / length, y / length, z };
	}

	// 거리 계산 (z 무시)
	float DistanceTo(const Vector3& other) const
	{
		float dx = x - other.x;
		float dy = y - other.y;
		return sqrtf(dx * dx + dy * dy);
	}
};
