#pragma once

#include "pch.h"
#include "Vector3.h"

class User;
class Monster;
class IOCP;

class Map
{
public:
	Map(int id, std::shared_ptr<IOCP> iocp);
	~Map();

public:
	void Update();

	void AddUser(unsigned int userID);
	void RemoveUser(unsigned int userID);

	unsigned int GetID() const;
	Vector3 GetUserSpawnPos() const;
	std::unordered_set<unsigned int> GetUsers() const;

private:
	void SpawnMonster();
	void MonsterStateUpdate();

private:
	unsigned int m_id;
	std::shared_ptr<IOCP> m_IOCP;
	std::unordered_set<unsigned int> m_users;
	std::unordered_map<unsigned int, std::shared_ptr<Monster>> m_monsters;
	Vector3 m_userSpawnPos;

	int m_responseCount;
};