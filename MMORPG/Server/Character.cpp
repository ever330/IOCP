#include "Character.h"
#include "MainServer.h"

Character::Character(unsigned int id, std::string name, unsigned int level, unsigned long exp)
	: m_characterID(id), m_name(name), m_level(level), m_exp(exp), m_maxHP(100), m_curHP(100), m_position(0, 0, 0), m_direction(0), m_isDirty(false)
{
	// Character 생성
}

void Character::Update(float deltaTime)
{
	if (m_isMoving)
	{
		Move(deltaTime);
	}
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

unsigned int Character::GetID() const
{
	return m_characterID;
}

unsigned int Character::GetMaxHp() const
{
	return m_maxHP;
}

unsigned int Character::GetCurHp() const
{
	return m_curHP;
}

void Character::SetPosition(Vector3 pos)
{
	m_position = pos;
}

Vector3 Character::GetPosition() const
{
	return m_position;
}

uint8_t Character::GetDirection() const
{
	return m_direction;
}

void Character::SetDirection(uint8_t dir)
{
	m_direction = dir;
}

void Character::SetMoving(bool moving)
{
	m_isMoving = moving;
}

void Character::AddExp(int exp)
{
	m_exp += exp;
	m_isDirty = true;
	while (m_exp >= GetRequiredExpForLevel(m_level))
	{
		m_exp -= GetRequiredExpForLevel(m_level);
		++m_level;
		MainServer::Instance().UserLevelSave(this);
	}
}

int Character::GetExp()
{
	return m_exp;
}

int Character::GetLevel()
{
	return m_level;
}

std::string Character::GetName() const
{
	return m_name;
}

bool Character::IsDirty() const
{
	return m_isDirty;
}

void Character::ClearDirty()
{
	m_isDirty = false;
}

void Character::Move(float deltaTime)
{
	float distance = m_speed * deltaTime; // 초당 속도 * 시간 = 이동 거리

	switch (m_direction)
	{
	case 0: // 상
		m_position.y -= distance;
		if (m_position.y < 10)
			m_position.y = 10;
		break;
	case 1: // 하
		m_position.y += distance;
		if (m_position.y > MAP_MAX_Y + 10)
			m_position.y = MAP_MAX_Y + 10;
		break;
	case 2: // 좌
		m_position.x -= distance;
		if (m_position.x < 10)
			m_position.x = 10;
		break;
	case 3: // 우
		m_position.x += distance;
		if (m_position.x > MAP_MAX_X + 10)
			m_position.x = MAP_MAX_X + 10;
		break;
	}
}

int Character::GetRequiredExpForLevel(int level)
{
	return 100 + (level - 1) * 50;
}
