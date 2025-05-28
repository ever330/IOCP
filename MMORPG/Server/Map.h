#pragma once

#include "pch.h"
#include "Util.h"
#include "Packet.h"

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

	void AddUser(std::shared_ptr<User> user);
	void RemoveUser(std::shared_ptr<User> user);

	unsigned int GetID() const;
	Vector3 GetUserSpawnPos() const;
	std::unordered_set<unsigned int> GetUsers() const;

	void PlayerAttack(std::shared_ptr<User> user, C2SPlayerAttackPacket pac);

private:
	void SpawnMonster();
	void MonsterStateUpdate();
	void PlayerStateUpdate();

private:
	unsigned int m_id;
	std::shared_ptr<IOCP> m_IOCP;
	std::unordered_set<unsigned int> m_users;
	std::unordered_map<unsigned int, std::shared_ptr<Monster>> m_monsters;
	Vector3 m_userSpawnPos;

	int m_responseCount;

	std::mutex m_mutex;
};