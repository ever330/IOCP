#pragma once

#include "pch.h"
#include "Vector3.h"
#include "Util.h"

class OtherPlayer
{
public:
	OtherPlayer() = delete;
	~OtherPlayer() = default;
    OtherPlayer(unsigned int id, CString name, Vector3 pos);

    void Update(float deltaTime);
    void Render(CDC* pDC);

    void SetPosition(float posX, float posY);
	Vector3 GetPosition() const;

	void SetTargetPosition(float posX, float posY);
	Vector3 GetTargetPosition() const;

	void SetDirection(Direction dir);
	Direction GetDirection() const;

	void SetName(CString name);
	CString GetName() const;

	void SetID(unsigned int id);
	unsigned int GetID() const;

private:
	unsigned int m_id;        // 플레이어 ID
	CString m_name;          // 플레이어 이름
    Vector3 m_position;        // 현재 위치 (화면에 그려질 위치)
	Vector3 m_targetPosition;  // 서버에서 보낸 위치
    Direction m_direction;    // 현재 방향
};