#include "Monster.h"
#include "Util.h"

Monster::Monster(int id, Vector3 spawnPos)
	: m_id(id), m_maxHP(100), m_curHP(100), m_position(spawnPos), m_direction(0)
{

}

Monster::~Monster()
{
	// Monster Á¦°Å
}

void Monster::Update()
{
	m_position.x += GetRandomFloat(-1.0f, 1.0f);
	m_position.y += GetRandomFloat(-1.0f, 1.0f);
}

void Monster::TakeDamage(int damage)
{
	m_curHP -= damage;
}

void Monster::Respawn(Vector3 spawnPos)
{
	m_curHP = m_maxHP;
	m_position = spawnPos;
}

bool Monster::IsDead() const
{
	return m_curHP <= 0;
}

unsigned int Monster::GetID() const
{
	return m_id;
}

unsigned int Monster::GetMaxHp() const
{
	return m_maxHP;
}

unsigned int Monster::GetCurHp() const
{
	return m_curHP;
}

Vector3 Monster::GetPosition() const
{
	return m_position;
}

uint8_t Monster::GetDirection() const
{
	return m_direction;
}
