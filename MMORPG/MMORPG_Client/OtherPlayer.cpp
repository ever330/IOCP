#include "OtherPlayer.h"

OtherPlayer::OtherPlayer(unsigned int id, CString name, Vector3 pos)
	: m_id(id), m_name(name), m_position(pos), m_targetPosition(-9999.9f, -9999.9f, 0.0f), m_direction(Direction::Down)
{

}

void OtherPlayer::Update(float deltaTime)
{
	if (m_targetPosition.x < -9000) return;

	float lerpSpeed = 8.0f;
	m_position.x += (m_targetPosition.x - m_position.x) * lerpSpeed * deltaTime;
	m_position.y += (m_targetPosition.y - m_position.y) * lerpSpeed * deltaTime;
}

void OtherPlayer::Render(CDC* pDC)
{
	int x = static_cast<int>(m_position.x);
	int y = static_cast<int>(m_position.y);

	// 글자 크기 측정
	CSize textSize = pDC->GetTextExtent(m_name);

	// 배경 사각형 출력
	CRect bgRect(x - 10, y - 25, x - 10 + textSize.cx, y - 25 + textSize.cy);
	pDC->FillSolidRect(bgRect, RGB(255, 255, 255));

	// 텍스트는 투명 배경으로 출력
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(x - 10, y - 25, m_name);
	pDC->FillSolidRect(x - 10, y - 10, 20, 20, RGB(0, 200, 0));
}

void OtherPlayer::SetPosition(float posX, float posY)
{
	m_position.x = posX;
	m_position.y = posY;
}

Vector3 OtherPlayer::GetPosition() const
{
	return m_position;
}

void OtherPlayer::SetTargetPosition(float posX, float posY)
{
	m_targetPosition.x = posX;
	m_targetPosition.y = posY;
}

Vector3 OtherPlayer::GetTargetPosition() const
{
	return m_targetPosition;
}

void OtherPlayer::SetDirection(Direction dir)
{
	m_direction = dir;
}

Direction OtherPlayer::GetDirection() const
{
	return m_direction;
}

void OtherPlayer::SetName(CString name)
{
	m_name = name;
}

CString OtherPlayer::GetName() const
{
	return m_name;
}

void OtherPlayer::SetID(unsigned int id)
{
	m_id = id;
}

unsigned int OtherPlayer::GetID() const
{
	return m_id;
}