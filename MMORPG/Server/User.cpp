#include "pch.h"
#include "User.h"

User::User(unsigned int userID, std::string userName) : m_userID(userID), m_userName(std::move(userName)), m_currentMapID(0), m_isConnected(false), m_lastInputFrame(0), m_isCharacterSet(false)
{

}

User::~User() 
{
	
}

void User::SetUserID(unsigned int userID) 
{
	m_userID = userID;
}

unsigned int User::GetUserID() const 
{
	return m_userID;
}

void User::SetUserName(const std::string& userName) 
{
	m_userName = userName;
}

std::string User::GetUserName() const 
{
	return m_userName;
}

void User::SetCurrentMapID(unsigned int mapID) 
{
	m_currentMapID = mapID;
}

unsigned int User::GetCurrentMapID() const 
{
	return m_currentMapID;
}

void User::SetCharacter(unsigned int id, std::string name, unsigned int level, unsigned long exp)
{
	m_character = std::make_unique<Character>(id, name, level, exp);
	m_isCharacterSet = true;
}

Character& User::GetCharacter()
{
	return *m_character;
}

bool User::IsCharacterSet() const
{
	return m_isCharacterSet;
}

void User::SetConnected(bool isConnected)
{
	m_isConnected = isConnected;
}

bool User::IsConnected() const
{
	return m_isConnected;
}

void User::SetLastInputFrame(unsigned int frame)
{
	m_lastInputFrame = frame;
}

unsigned int User::GetLastInputFrame() const
{
	return m_lastInputFrame;
}
