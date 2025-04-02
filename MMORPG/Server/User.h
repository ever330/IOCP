#pragma once

#include "pch.h"

class User
{
public:
	User() = delete;
	~User() {}
	User(unsigned int userID, std::string userName) : m_userID(userID), m_userName(userName) {}

public:
	void SetUserID(unsigned int userID) { m_userID = userID; }
	unsigned int GetUserID() { return m_userID; }

	void SetUserName(std::string userName) { m_userName = userName; }
	std::string GetUserName() { return m_userName; }

private:
	unsigned int m_userID;
	std::string m_userName;
};