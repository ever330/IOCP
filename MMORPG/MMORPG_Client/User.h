#pragma once

#include "pch.h"

class User
{
public:
	const unsigned int GetUserId() const { return m_userid; }
	const std::string& GetUsername() const { return m_username; }
	void SetUserId(unsigned int userid) { m_userid = userid; }
	void SetUsername(const std::string& username) { m_username = username; }

private:
	uint16_t m_userid;
	std::string m_username;
};