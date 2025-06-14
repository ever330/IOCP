#pragma once

#include "pch.h"
#include "Character.h"

class User
{
public:
	User() = delete;
	~User();
	User(unsigned int userID, std::string userName);

public:
	void SetUserID(unsigned int userID);
	unsigned int GetUserID() const;

	void SetUserName(const std::string& userName);
	std::string GetUserName() const;

	void SetCurrentMapID(unsigned int mapID);
	unsigned int GetCurrentMapID() const;

	Character& GetCharacter();

private:
	unsigned int m_userID;
	std::string m_userName;
	unsigned int m_currentMapID;
	Character m_character;
};