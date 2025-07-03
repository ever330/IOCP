#pragma once

#include "pch.h"
#include "Vector3.h"
#include "Define.h"

class Character
{
public:
	Character() = delete;
	~Character() = default;
	Character(unsigned int id, std::string name, unsigned int level, unsigned long exp);

	void Update(float deltaTime);
	void TakeDamage(int damage);
	void Respawn(Vector3 spawnPos);
	bool IsDead() const;

	unsigned int GetID() const;

	unsigned int GetMaxHp() const;
	unsigned int GetCurHp() const;

	void SetPosition(Vector3 pos);
	Vector3 GetPosition() const;

	uint8_t GetDirection() const;
	void SetDirection(uint8_t dir);
	void SetMoving(bool moving);

	void AddExp(int exp);
	int GetExp();
	
	int GetLevel();

	std::string GetName() const;

	bool IsDirty() const;
	void ClearDirty();

private:
	void Move(float deltaTime);
	int GetRequiredExpForLevel(int level);

	unsigned int m_characterID;
	std::string m_name;
	unsigned int m_level;
	unsigned long m_exp;
	unsigned int m_maxHP;
	unsigned int m_curHP;
	Vector3 m_position;
	uint8_t m_direction;

	bool m_isMoving = false;
	float m_speed = 150.0f;

	// DB 업데이트를 위한 플래그
	bool m_isDirty;

	const int m_needExpForLevelUp = 100;
};