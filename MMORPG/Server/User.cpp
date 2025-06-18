#include "User.h"

User::User(unsigned int userID, std::string userName) : m_userID(userID), m_userName(std::move(userName)), m_currentMapID(0), m_character({ 0.0f, 0.0f, 0.0f }), m_isConnected(false)
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

Character& User::GetCharacter() 
{
	return m_character;
}

void User::SetConnected(bool isConnected)
{
	m_isConnected = isConnected;
}

bool User::IsConnected() const
{
	return m_isConnected;
}
