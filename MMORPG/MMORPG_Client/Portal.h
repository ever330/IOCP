#pragma once

#include "pch.h"
#include "Vector3.h"

class Portal
{
public:
	Portal() = delete;
	~Portal() = default;
	Portal(const unsigned int id, const Vector3 position, const unsigned int targetMapID);

	void Render(CDC* pDC);
	bool OnEnter(const Vector3 pos) const;
	unsigned int GetID() const;

private:
	unsigned int m_id;
	Vector3 m_position;
	unsigned int m_targetMapID;
};