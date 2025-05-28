#include "Monster.h"
#include "Util.h"

void Monster::ApplyState(const S2CMonsterStateInfo& state)
{
    m_monsterID = state.MonsterID;
    m_spawnID = state.SpawnID;
	m_pos.x = state.PosX;
	m_pos.y = state.PosY;
	m_pos.z = state.PosZ;
    m_curHP = state.CurHP;
}

void Monster::Update(float deltaTime)
{
    if (m_curHP <= 0)
    {
        return; // HP�� 0 ������ ��� ������Ʈ���� ����
	}

    if (m_isKnockback)
    {
        m_knockbackTimer += deltaTime;
        float t = m_knockbackTimer / m_knockbackDuration;
        if (t >= 1.0f)
        {
            m_isKnockback = false;
            t = 1.0f;
        }

        m_pos.x = m_originX + (m_targetX - m_originX) * t;
        m_pos.y = m_originY + (m_targetY - m_originY) * t;
    }
}

void Monster::Render(CDC* pDC)
{
    if (m_curHP <= 0)
    {
        return; // HP�� 0 ������ ��� ���������� ����
	}

    int x = static_cast<int>(m_pos.x);
    int y = static_cast<int>(m_pos.y);

    pDC->Ellipse(x - 10, y - 10, x + 10, y + 10);

    // ü�¹� ���
    const int barWidth = 40;
    const int barHeight = 6;
    const int barX = x - barWidth / 2;
    const int barY = y - 20;

    double ratio = static_cast<double>(m_curHP) / static_cast<double>(m_maxHP);
    int hpWidth = static_cast<int>(barWidth * ratio);

    // ü�� �� ��� (ȸ��)
    pDC->FillSolidRect(barX, barY, barWidth, barHeight, RGB(200, 200, 200));

    // ü�� �� (������)
    pDC->FillSolidRect(barX, barY, hpWidth, barHeight, RGB(255, 0, 0));
}

void Monster::ApplyHit(const S2CMonsterHitInfo& hit)
{
    m_curHP -= hit.Damage;
    if (m_curHP <= 0)
    {
        m_curHP = 0;
    }

    // �˹� �ʱ�ȭ
    m_isKnockback = true;
    m_knockbackTimer = 0.0f;

    // ���� ��ġ ����
    m_originX = m_pos.x;
    m_originY = m_pos.y;

    // ���� �������� �˹� ��� ��ġ ���
    const float knockbackDistance = 40.0f;

    switch ((Direction)hit.AttackDirection) 
    {
    case Direction::Left:
        m_targetX = m_originX - knockbackDistance;
        m_targetY = m_originY;
        break;
    case Direction::Right:
        m_targetX = m_originX + knockbackDistance;
        m_targetY = m_originY;
        break;
    case Direction::Up:
        m_targetX = m_originX;
        m_targetY = m_originY - knockbackDistance;
        break;
    case Direction::Down:
        m_targetX = m_originX;
        m_targetY = m_originY + knockbackDistance;
        break;
    default:
        m_targetX = m_originX;
        m_targetY = m_originY;
        break;
    }
}

void Monster::Respawn(const Vector3& pos, uint16_t maxHP, uint16_t curHP)
{
    m_pos.x = pos.x;
    m_pos.y = pos.y;
    m_pos.z = pos.z;

    m_maxHP = maxHP;
    m_curHP = curHP;

    m_isKnockback = false;
    m_knockbackTimer = 0.0f;

    // ���� ���� ����
    m_isDead = false; // isDead()���� �����ϴ� �÷���
}

void Monster::SetID(uint16_t monsterID)
{
	m_monsterID = monsterID;
}
