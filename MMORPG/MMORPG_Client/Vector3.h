#pragma once

#include "pch.h"

struct Vector3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	Vector3() = default;

	Vector3(float x, float y, float z)
		: x(x), y(y), z(z) {}

	// ����
	Vector3 operator+(const Vector3& other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	// ����
	Vector3 operator-(const Vector3& other) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	// ��Į�� ��
	Vector3 operator*(float scalar) const
	{
		return { x * scalar, y * scalar, z * scalar };
	}

	// ���� ����ȭ
	Vector3 Normalize() const
	{
		float length = sqrtf(x * x + y * y); // z ����
		if (length == 0.0f)
			return { 0.0f, 0.0f, 0.0f };
		return { x / length, y / length, z };
	}

	// �Ÿ� ��� (z ����)
	float DistanceTo(const Vector3& other) const
	{
		float dx = x - other.x;
		float dy = y - other.y;
		return sqrtf(dx * dx + dy * dy);
	}
};