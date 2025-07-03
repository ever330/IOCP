#pragma once

#include "pch.h"
#include "Util.h"
#include "Vector3.h"

class User;

class Monster
{
public:
	Monster(int id, Vector3 spawnPos);
	~Monster();

	void Update(const std::unordered_set<unsigned int> mapUsers);
	std::optional<int> TakeDamage(Direction dir, int damage, int knockbackDistance);
	void Respawn(Vector3 spawnPos);

	bool IsDead() const;
	unsigned int GetID() const;
	unsigned int GetMaxHp() const;
	unsigned int GetCurHp() const;
	Vector3 GetPosition() const;

private:
	std::shared_ptr<User> FindClosetUser(const std::unordered_set<unsigned int> mapUsers);
	void FreeWalk();

	int m_id;
	int m_maxHP;
	int m_curHP;
	Vector3 m_position;
	bool m_isDead;
	
	const int m_speed = 5;
	const int m_detectRange = 100;
	const int m_expReward = 10;
};

