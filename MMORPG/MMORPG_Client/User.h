#pragma once

#include "packet.h"

enum class Gender : uint8_t 
{
	Male = 0,
	Female = 1
};

struct Character
{
	unsigned int ID;
	std::string Name;
	Gender GenderType;
	unsigned int Level;
	unsigned long Experience;
};

class User
{
public:
	const unsigned int GetUserId() const;
	void SetUserId(unsigned int userid);

	const std::string& GetUsername() const;
	void SetUsername(const std::string& username);

	void SetActiveCharacter(const Character& character);

	void AddCharacter(const Character& character);
	void SetCharacters(const std::vector<S2CCharacterInfo>& characters);
	const std::unordered_map<unsigned int, Character>& GetCharacters() const;
	const Character& GetActiveCharacter() const;

	void SetExperience(unsigned long experience);
	void SetLevel(unsigned int level);

private:
	uint16_t m_userid;
	std::string m_username;
	Character m_activeCharacter;
	std::unordered_map<unsigned int, Character> m_characters;
};