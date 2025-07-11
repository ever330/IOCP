#pragma once

#include "pch.h"
#include "Util.h"
#include "Packet.h"

class User;
class Monster;

struct Portal
{
	unsigned int ID;
	unsigned int TargetMapID;
	Vector3 Position;
	Vector3 SpawnPosition;
};

class Map
{
public:
	Map(int id);
	~Map();

	void Initialize();
	void Update(float deltaTime, int tickCount);

	void AddPortal(unsigned int targetMapID, Vector3 position, Vector3 spawnPosition);
	Portal GetPortal(unsigned int id) const;
	std::unordered_map<unsigned int, Portal> GetPortals() const;
	unsigned int GetPortalCount() const;

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

	unsigned int m_id;
	std::unordered_set<unsigned int> m_users;
	std::unordered_map<unsigned int, std::shared_ptr<Monster>> m_monsters;
	Vector3 m_userSpawnPos;

	std::unordered_map<unsigned int, Portal> m_portals;

	std::mutex m_mutex;

	const int m_monsterSpawnCount = 10;
};