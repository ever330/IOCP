#include "Portal.h"

Portal::Portal(const unsigned int id, const Vector3 position, const unsigned int targetMapID)
	: m_id(id), m_position(position), m_targetMapID(targetMapID) 
{
}

void Portal::Render(CDC* pDC)
{
	int x = static_cast<int>(m_position.x);
	int y = static_cast<int>(m_position.y);

	pDC->FillSolidRect(x - 15, y - 15, 30, 30, RGB(173, 216, 230));
}

bool Portal::OnEnter(const Vector3 pos) const
{
	if (pos.x >= m_position.x - 15.0f && pos.x <= m_position.x + 15.0f &&
		pos.y >= m_position.y - 15.0f && pos.y <= m_position.y + 15.0f)
	{
		return true;
	}
	return false;
}

unsigned int Portal::GetID() const
{
	return m_id;
}