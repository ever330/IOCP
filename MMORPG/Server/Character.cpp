#include "Character.h"

Character::Character(Vector3 spawnPos)
	: m_maxHP(100), m_curHP(100), m_position(spawnPos), m_direction(0)
{
	// Character 생성
}

Character::~Character()
{
	// Character 제거
}

void Character::Update()
{

}

void Character::TakeDamage(int damage)
{
	m_curHP -= damage;
}

void Character::Respawn(Vector3 spawnPos)
{
	m_curHP = m_maxHP;
	m_position = spawnPos;
}

bool Character::IsDead() const
{
	return m_curHP <= 0;
}

unsigned int Character::GetMaxHp() const
{
	return m_maxHP;
}

unsigned int Character::GetCurHp() const
{
	return m_curHP;
}

Vector3 Character::GetPosition() const
{
	return m_position;
}

uint8_t Character::GetDirection() const
{
	return m_direction;
}

void Character::Move(int dir)
{
	if (dir == 0)
	{
		m_position.x -= 10.0f;
		if (m_position.x < 10.0f)
		{
			m_position.x = 10.0f;
		}
	}
	else if (dir == 1)
	{
		m_position.x += 10.0f;
		if (m_position.x > MAP_MAX_X + 10.0f)
		{
			m_position.x = MAP_MAX_X + 10.0f;
		}
	}
	else if (dir == 2)
	{
		m_position.y -= 10.0f;
		if (m_position.y < 10.0f)
		{
			m_position.y = 10.0f;
		}
	}
	else if (dir == 3)
	{
		m_position.y += 10.0f;
		if (m_position.y > MAP_MAX_Y + 10.0f)
		{
			m_position.y = MAP_MAX_Y + 10.0f;
		}
	}
}