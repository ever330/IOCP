#pragma once

#include "pch.h"
#include "Vector3.h"

class Character
{
public:
	Character() = delete;
	~Character();
	Character(Vector3 spawnPos);

public:
	void Update();
	void TakeDamage(int damage);
	void Respawn(Vector3 spawnPos);
	bool IsDead() const;
	unsigned int GetMaxHp() const;
	unsigned int GetCurHp() const;
	Vector3 GetPosition() const;
	uint8_t GetDirection() const;
	void Move(int dir);

private:
	unsigned int m_maxHP;
	unsigned int m_curHP;
	Vector3 m_position;
	uint8_t m_direction;
};