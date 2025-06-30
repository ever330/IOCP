#include "Character.h"
#include "MainServer.h"

Character::Character(Vector3 spawnPos)
	: m_maxHP(100), m_curHP(100), m_position(spawnPos), m_direction(0)
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

void Character::SetDirection(uint8_t dir)
{
	m_direction = dir;
}

void Character::SetMoving(bool moving)
{
	m_isMoving = moving;
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

	std::string logMessage = "Character moved to position: (" + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ")";
	MainServer::Instance().Log(logMessage);
}