#pragma once

#include "pch.h"
#include "Character.h"

class User
{
public:
	User() = delete;
	~User() {}
	User(unsigned int userID, std::string userName) : m_userID(userID), m_userName(userName), m_currentMapID(0), m_character({ 0.0f, 0.0f, 0.0f }) {}

public:
	void SetUserID(unsigned int userID) { m_userID = userID; }
	unsigned int GetUserID() { return m_userID; }

	void SetUserName(std::string userName) { m_userName = userName; }
	std::string GetUserName() { return m_userName; }

	void SetCurrentMapID(unsigned int mapID) { m_currentMapID = mapID; }
	unsigned int GetCurrentMapID() { return m_currentMapID; }

	Character& GetCharacter() { return m_character; }

private:
	unsigned int m_userID;
	std::string m_userName;
	unsigned int m_currentMapID;
	Character m_character;
};