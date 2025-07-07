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
	unsigned int m_id;        // �÷��̾� ID
	CString m_name;          // �÷��̾� �̸�
    Vector3 m_position;        // ���� ��ġ (ȭ�鿡 �׷��� ��ġ)
	Vector3 m_targetPosition;  // �������� ���� ��ġ
    Direction m_direction;    // ���� ����
};