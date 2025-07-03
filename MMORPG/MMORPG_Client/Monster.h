#pragma once

#include "pch.h"
#include "packet.h"
#include "Vector3.h"

class Monster
{
public:
    void ApplyState(const S2CMonsterStateInfo& state);
    void Update(float deltaTime);
    void Render(CDC* pDC);
	void ApplyHit(const S2CMonsterHitInfo& hit);
    void Respawn(const Vector3& pos, uint16_t maxHP, uint16_t curHP);
	bool IsDead() const { return m_isDead; }
    void SetID(uint16_t monsterID);

private:
    uint16_t m_monsterID;
    uint16_t m_spawnID;

    Vector3 m_pos;

    uint16_t m_maxHP;
    uint16_t m_curHP;
    
	bool m_isDead = false;

    // 넉백, 애니메이션 등에 사용될 상태들
    bool m_isKnockback = false;
    float m_knockbackTimer = 0.0f;
    float m_knockbackDuration = 0.3f;
    float m_originX;
    float m_originY;
    float m_targetX;
    float m_targetY;
};

