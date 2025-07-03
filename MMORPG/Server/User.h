#pragma once

#include "pch.h"
#include "Character.h"

class User
{
public:
	User() = delete;
	~User();
	User(unsigned int userID, std::string userName);

	void SetUserID(unsigned int userID);
	unsigned int GetUserID() const;

	void SetUserName(const std::string& userName);
	std::string GetUserName() const;

	void SetCurrentMapID(unsigned int mapID);
	unsigned int GetCurrentMapID() const;

	void SetCharacter(unsigned int id, std::string name, unsigned int level, unsigned long exp);
	Character& GetCharacter();

	bool IsCharacterSet() const;

	void SetConnected(bool isConnected);
	bool IsConnected() const;

	void SetLastInputFrame(unsigned int frame);
	unsigned int GetLastInputFrame() const;

private:
	unsigned int m_userID;
	std::string m_userName;
	unsigned int m_currentMapID;
	std::unique_ptr<Character> m_character;
	bool m_isConnected;
	unsigned int m_lastInputFrame;
	bool m_isCharacterSet = false;
};