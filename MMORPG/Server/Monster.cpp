#include "Monster.h"
#include "Util.h"
#include "Define.h"
#include "User.h"
#include "MainServer.h"

Monster::Monster(int id, Vector3 spawnPos)
	: m_id(id), m_maxHP(100), m_curHP(100), m_position(spawnPos), m_isDead(false)
{

}

Monster::~Monster()
{
	// Monster ����
}

void Monster::Update(const std::unordered_set<unsigned int> mapUsers)
{
	std::shared_ptr<User> target = FindClosetUser(mapUsers);

	if (target)
	{
		Vector3 dir = target->GetCharacter().GetPosition() - m_position;

		// �밢�� �̵� ����: �� �ุ �̵�
		if (fabs(dir.x) > fabs(dir.y))
		{
			// x �������θ� �̵�
			m_position.x += (dir.x > 0 ? 1.0f : -1.0f) * m_speed;
		}
		else
		{
			// y �������θ� �̵�
			m_position.y += (dir.y > 0 ? 1.0f : -1.0f) * m_speed;
		}
	}
	else
	{
		FreeWalk();
	}
}

std::optional<int> Monster::TakeDamage(Direction dir, int damage, int knockbackDistance)
{
	m_curHP -= damage;
	if (m_curHP <= 0)
	{
		m_curHP = 0;
	}

	if (dir == Direction::Left || dir == Direction::Right)
	{
		// �¿� ���� ���� �� x������ �з���
		m_position.x += (dir == Direction::Left ? -1.0f : 1.0f) * knockbackDistance;
	}
	else if (dir == Direction::Up || dir == Direction::Down)
	{
		// ���� ���� ���� �� y������ �з���
		m_position.y += (dir == Direction::Up ? -1.0f : 1.0f) * knockbackDistance;
	}

	if (m_curHP <= 0 && !m_isDead)
	{
		m_isDead = true;
		return m_expReward;  // ��: ���ͺ� ����ġ ����
	}

	return std::nullopt;
}

void Monster::Respawn(Vector3 spawnPos)
{
	m_curHP = m_maxHP;
	m_position = spawnPos;
	m_isDead = false;
}

bool Monster::IsDead() const
{
	return m_isDead;
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

std::shared_ptr<User> Monster::FindClosetUser(const std::unordered_set<unsigned int> mapUsers)
{
	std::shared_ptr<User> closestUser = nullptr;

	float closestDistance = m_detectRange;

	for (const auto& userID : mapUsers)
	{
		auto user = MainServer::Instance().GetUserByID(userID);
		if (!user) continue;

		float dx = user->GetCharacter().GetPosition().x - m_position.x;
		float dy = user->GetCharacter().GetPosition().y - m_position.y;
		float distance = sqrtf(dx * dx + dy * dy);

		if (distance < m_detectRange)
		{
			closestDistance = distance;
			closestUser = user;
		}
	}

	return closestUser;
}

void Monster::FreeWalk()
{
	// x �Ǵ� y �� �ϳ��� �����ϰ� ����
	bool moveX = GetRandomInt(0, 1) == 0;

	// �̵� ���� ����: -1 �Ǵ� +1
	int dir = (GetRandomInt(0, 1) == 0) ? -1 : 1;

	if (moveX)
	{
		float nextX = m_position.x + dir * m_speed;

		// �� ���� üũ
		if (nextX < 0.0f || nextX > MAP_MAX_X)
		{
			// ���� ����
			dir *= -1;
			nextX = m_position.x + dir * m_speed;
		}

		m_position.x = nextX;
	}
	else
	{
		float nextY = m_position.y + dir * m_speed;

		if (nextY < 0.0f || nextY > MAP_MAX_Y)
		{
			dir *= -1;
			nextY = m_position.y + dir * m_speed;
		}

		m_position.y = nextY;
	}
}
