#pragma once

#include "pch.h"
#include "Vector3.h"
#include "Define.h"

class Character
{
public:
	Character() = delete;
	~Character() = default;
	Character(Vector3 spawnPos);

	void Update(float deltaTime);
	void TakeDamage(int damage);
	void Respawn(Vector3 spawnPos);
	bool IsDead() const;
	unsigned int GetMaxHp() const;
	unsigned int GetCurHp() const;
	Vector3 GetPosition() const;
	uint8_t GetDirection() const;

	void SetDirection(uint8_t dir);
	void SetMoving(bool moving);

private:
	void Move(float deltaTime);

	unsigned int m_maxHP;
	unsigned int m_curHP;
	Vector3 m_position;
	uint8_t m_direction;

	bool m_isMoving = false;
	float m_speed = 150.0f;
};