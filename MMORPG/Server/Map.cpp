#include "Map.h"
#include "User.h"
#include "Monster.h"
#include "IOCP.h"
#include "MainServer.h"
#include "Util.h"

Map::Map(int id, std::shared_ptr<IOCP> iocp)
	: m_id(id), m_IOCP(iocp), m_responseCount(0)
{
	// 몬스터 수 임시로 5마리 생성.
	for (int i = 1; i <= 5; ++i)
	{
		// 임시 스폰 위치. Map 관련 DB가 있을 경우, 각 맵 별로 몬스터 스폰위치가 정해져야 함
		Vector3 spawnPos = { GetRandomFloat(1.0f, 10.0f), GetRandomFloat(1.0f, 10.0f), 0.0f};
		m_monsters.insert({ i, std::make_shared<Monster>(1, spawnPos) });
	}

	// 임시 유저 스폰 위치.
	m_userSpawnPos = { 5.0f, 5.0f, 0.0f };
}

Map::~Map()
{
	// 맵 제거
}

void Map::Update()
{
	// 몬스터 업데이트
	for (auto& monster : m_monsters)
	{
		monster.second->Update();
	}

	MonsterStateUpdate();

	// 몬스터 스폰 주기, Update 주기인 0.1초 * 횟수
	if (m_responseCount >= SPAWN_COUNT)
	{
		SpawnMonster();
		m_responseCount = 0;
	}

	m_responseCount++;
}

void Map::AddUser(unsigned int userID)
{
	auto it = m_users.find(userID);
	if (it == m_users.end())
	{
		S2CPlayerEnterPacket enterPacket;
		enterPacket.UserID = userID;
		memcpy(enterPacket.Name, MainServer::Instance().GetUser(userID)->GetUserName().c_str(), sizeof(enterPacket.Name));
		enterPacket.SpawnPosX = m_userSpawnPos.x;
		enterPacket.SpawnPosY = m_userSpawnPos.y;
		enterPacket.SpawnPosY = 0.0f;

		PacketBase* packet = (PacketBase*)new char[sizeof(PacketBase) + sizeof(S2CPlayerEnterPacket)];

		packet->PacketSize = sizeof(PacketBase) + sizeof(S2CPlayerEnterPacket);
		packet->PacID = PacketID::S2CPlayerEnter;
		memcpy(packet->Body, &enterPacket, sizeof(S2CPlayerEnterPacket));

		if (!m_users.empty())
			MainServer::Instance().BroadCast(m_users, packet);

		delete[](char*)packet;

		m_users.insert(userID);
	}
}

void Map::RemoveUser(unsigned int userID)
{
	auto it = m_users.find(userID);
	if (it != m_users.end())
	{
		m_users.erase(it);

		S2CPlayerLeavePacket leavePacket;
		memcpy(leavePacket.Name, MainServer::Instance().GetUser(userID)->GetUserName().c_str(), sizeof(leavePacket.Name));

		PacketBase* packet = (PacketBase*)new char[sizeof(PacketBase) + sizeof(S2CPlayerLeavePacket)];

		packet->PacketSize = sizeof(PacketBase) + sizeof(S2CPlayerLeavePacket);
		packet->PacID = PacketID::S2CPlayerLeave;
		memcpy(packet->Body, &leavePacket, sizeof(S2CPlayerLeavePacket));

		if (!m_users.empty())
			MainServer::Instance().BroadCast(m_users, packet);

		delete[](char*)packet;
	}
}

unsigned Map::GetID() const
{
	return m_id;
}

Vector3 Map::GetUserSpawnPos() const
{
	return m_userSpawnPos;
}

std::unordered_set<unsigned int> Map::GetUsers() const
{
	return m_users;
}

void Map::SpawnMonster()
{
	std::vector<MonsterRespawnInfo> respawnMonsters;

	for (auto& monster : m_monsters)
	{
		if (monster.second->IsDead())
		{
			// 몬스터 리스폰
			Vector3 spawnPos = { 0.0f, 0.0f, 0.0f };
			monster.second->Respawn(spawnPos);

			MonsterRespawnInfo info;
			info.MonsterID = monster.second->GetID();
			info.SpawnID = monster.first;
			info.SpawnPosX = spawnPos.x;
			info.SpawnPosY = spawnPos.y;
			info.SpawnPosZ = spawnPos.z;
			info.MaxHP = monster.second->GetMaxHp();
			info.CurHP = monster.second->GetCurHp();

			respawnMonsters.push_back(info);
		}
	}

	if (!respawnMonsters.empty())
	{
		const uint16_t monsterCount = static_cast<uint16_t>(respawnMonsters.size());

		const uint16_t packetSize = sizeof(PacketBase)
			+ sizeof(S2CMonsterRespawnPacket)
			+ sizeof(MonsterRespawnInfo) * monsterCount;

		PacketBase* packet = reinterpret_cast<PacketBase*>(new char[packetSize]);

		packet->PacketSize = packetSize;
		packet->PacID = S2CMonsterRespawn;

		S2CMonsterRespawnPacket* header = reinterpret_cast<S2CMonsterRespawnPacket*>(packet->Body);
		header->MonsterCount = monsterCount;

		MonsterRespawnInfo* monsters = reinterpret_cast<MonsterRespawnInfo*>(header + 1);
		memcpy(monsters, respawnMonsters.data(), sizeof(MonsterRespawnInfo) * monsterCount);

		if (!m_users.empty())
			MainServer::Instance().BroadCast(m_users, packet);

        delete[] reinterpret_cast<char*>(packet);
	}
}

void Map::MonsterStateUpdate()
{
	std::vector<S2CMonsterStateInfo> monsterStates;
	for (auto& monster : m_monsters)
	{
		if (!monster.second->IsDead())
		{
			S2CMonsterStateInfo info;
			info.MonsterID = monster.second->GetID();
			info.SpawnID = monster.first;
			info.PosX = monster.second->GetPosition().x;
			info.PosY = monster.second->GetPosition().y;
			info.PosZ = monster.second->GetPosition().z;
			info.Direction = monster.second->GetDirection();
			monsterStates.push_back(info);
		}
	}
	if (!monsterStates.empty())
	{
		const uint16_t monsterCount = static_cast<uint16_t>(monsterStates.size());
		const uint16_t packetSize = sizeof(PacketBase)
			+ sizeof(S2CMonsterStatePacket)
			+ sizeof(S2CMonsterStateInfo) * monsterCount;
		PacketBase* packet = reinterpret_cast<PacketBase*>(new char[packetSize]);

		packet->PacketSize = packetSize;
		packet->PacID = S2CMonsterState;

		S2CMonsterStatePacket* header = reinterpret_cast<S2CMonsterStatePacket*>(packet->Body);
		header->MonsterCount = monsterCount;

		S2CMonsterStateInfo* monsters = reinterpret_cast<S2CMonsterStateInfo*>(header + 1);
		memcpy(monsters, monsterStates.data(), sizeof(S2CMonsterStateInfo) * monsterCount);

		if (!m_users.empty())
			MainServer::Instance().BroadCast(m_users, packet);

		delete[] reinterpret_cast<char*>(packet);
	}
}
