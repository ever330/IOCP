#pragma once

#include "pch.h"
#include "Vector3.h"

class Monster
{
public:
	Monster(int id, Vector3 spawnPos);
	~Monster();

public:
	void Update();
	void TakeDamage(int damage);
	void Respawn(Vector3 spawnPos);

	bool IsDead() const;
	unsigned int GetID() const;
	unsigned int GetMaxHp() const;
	unsigned int GetCurHp() const;
	Vector3 GetPosition() const;
	uint8_t GetDirection() const;

private:
	int m_id;
	int m_maxHP;
	int m_curHP;
	Vector3 m_position;
	uint8_t m_direction;	// 0 : ¿ÞÂÊ, 1 : ¿À¸¥ÂÊ
};

