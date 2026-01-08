#include "pch.h"
#include "User.h"

const unsigned int User::GetUserId() const
{
	return m_userid;
}

void User::SetUserId(unsigned int userid)
{
	m_userid = userid;
}

const std::string& User::GetUsername() const
{
	return m_username;
}

void User::SetUsername(const std::string& username)
{
	m_username = username;
}

void User::SetActiveCharacter(const Character& character)
{
	m_activeCharacter = character;
}

void User::AddCharacter(const Character& character)
{
	m_characters[character.ID] = character;
}

void User::SetCharacters(const std::vector<S2CCharacterInfo>& characters)
{
	for (const auto& character : characters)
	{
		Character newCharacter;
		newCharacter.ID = character.CharacterID;
		newCharacter.Name = character.Name;
		newCharacter.GenderType = (Gender)character.Gender;
		newCharacter.Level = character.Level;
		newCharacter.Experience = character.Exp;
		m_characters[character.CharacterID] = newCharacter;
	}
}

const std::unordered_map<unsigned int, Character>& User::GetCharacters() const
{
	return m_characters;
}

const Character& User::GetActiveCharacter() const
{
	return m_activeCharacter;
}

void User::SetExperience(unsigned long experience)
{
	m_activeCharacter.Experience = experience;
}

void User::SetLevel(unsigned int level)
{
	m_activeCharacter.Level = level;
}