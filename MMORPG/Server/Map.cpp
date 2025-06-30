#include "Map.h"
#include "User.h"
#include "Monster.h"
#include "MainServer.h"

Map::Map(int id)
	: m_id(id), m_responseCount(0)
{
}

Map::~Map()
{
	// 맵 제거
}

void Map::Initialize()
{
	for (int i = 1; i <= m_monsterSpawnCount; ++i)
	{
		// 임시 스폰 위치. Map 관련 DB가 있을 경우, 각 맵 별로 몬스터 스폰위치가 정해져야 함
		Vector3 spawnPos = { GetRandomFloat(0.0f, MAP_MAX_X), GetRandomFloat(0.0f, MAP_MAX_Y), 0.0f };
		m_monsters.insert({ i, std::make_shared<Monster>(1, spawnPos) });
	}
	// 임시 유저 스폰 위치.
	m_userSpawnPos.x = 100.0f;
	m_userSpawnPos.y = 100.0f;
}

void Map::Update(float deltaTime, int tickCount)
{
	// 유저 업데이트
	for (int userID : m_users) {
		std::shared_ptr<User> user = MainServer::Instance().GetUserByID(userID);

		if (!user)
			continue;

		user->GetCharacter().Update(deltaTime); // 방향에 따라 좌표 이동
	}

	// 몬스터 업데이트
	for (auto& monster : m_monsters)
	{
		monster.second->Update(m_users);
	}

	// 10틱마다 유저 좌표 상태 브로드캐스트
	if (tickCount % 10 == 0) {
		PlayerStateUpdate();  // 유저 좌표 상태 전체 브로드캐스트
	}

	MonsterStateUpdate();

	if (m_responseCount >= SPAWN_COUNT)
	{
		SpawnMonster();
		m_responseCount = 0;
	}

	m_responseCount++;
}

void Map::AddUser(std::shared_ptr<User> user)
{
	std::scoped_lock lock(m_mutex);

	if (m_users.insert(user->GetUserID()).second)
	{
		S2CPlayerEnterPacket enterPacket;
		enterPacket.UserID = user->GetUserID();
		memcpy(enterPacket.Name, user->GetUserName().c_str(), sizeof(enterPacket.Name));
		enterPacket.SpawnPosX = m_userSpawnPos.x;
		enterPacket.SpawnPosY = m_userSpawnPos.y;
		enterPacket.SpawnPosZ = 0.0f;

		PacketBase* packet = (PacketBase*)new char[sizeof(PacketBase) + sizeof(S2CPlayerEnterPacket)];

		packet->PacketSize = sizeof(PacketBase) + sizeof(S2CPlayerEnterPacket);
		packet->PacID = PacketID::S2CPlayerEnter;
		memcpy(packet->Body, &enterPacket, sizeof(S2CPlayerEnterPacket));

		if (m_users.size() > 1)
			MainServer::Instance().BroadCast(m_users, packet);

		delete[] reinterpret_cast<char*>(packet);
	}
}

void Map::RemoveUser(std::shared_ptr<User> user)
{
	std::scoped_lock lock(m_mutex);

	unsigned int userID = user->GetUserID();
	auto it = m_users.find(userID);
	if (it != m_users.end())
	{
		m_users.erase(it);

		S2CPlayerLeavePacket leavePacket;
		leavePacket.UserID = userID;
		memcpy(leavePacket.Name, user->GetUserName().c_str(), sizeof(leavePacket.Name));

		PacketBase* packet = (PacketBase*)new char[sizeof(PacketBase) + sizeof(S2CPlayerLeavePacket)];

		packet->PacketSize = sizeof(PacketBase) + sizeof(S2CPlayerLeavePacket);
		packet->PacID = PacketID::S2CPlayerLeave;
		memcpy(packet->Body, &leavePacket, sizeof(S2CPlayerLeavePacket));

		if (!m_users.empty())
			MainServer::Instance().BroadCast(m_users, packet);

		delete[] reinterpret_cast<char*>(packet);
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

void Map::PlayerAttack(std::shared_ptr<User> user, C2SPlayerAttackPacket pac)
{
	const uint16_t ackPacSize = sizeof(PacketBase)
		+ sizeof(S2CPlayerAttackPacket);
	std::shared_ptr<char[]> ackBuf(new char[ackPacSize]);
	PacketBase* ackPac = reinterpret_cast<PacketBase*>(ackBuf.get());
	ackPac->PacketSize = ackPacSize;
	ackPac->PacID = S2CPlayerAttack;
	S2CPlayerAttackPacket* attackAck = reinterpret_cast<S2CPlayerAttackPacket*>(ackPac->Body);
	attackAck->UserID = user.get()->GetUserID();
	attackAck->AttackDirection = pac.AttackDirection;

	MainServer::Instance().BroadCast(m_users, ackPac);

	Vector3 playerPos = user.get()->GetCharacter().GetPosition();
	Direction dir = (Direction)pac.AttackDirection; // 패킷에 포함되어 있어야 함

	AttackRect hitBox = GetAttackRect(playerPos, dir);

	std::vector<S2CMonsterHitInfo> hitMonsters;

	for (auto& monster : m_monsters)
	{
		if (monster.second.get()->IsDead())
			continue;

		Vector3 monPos = monster.second.get()->GetPosition();

		if (hitBox.Contains(monPos))
		{
			monster.second.get()->TakeDamage(dir, 20, 40);
			
			S2CMonsterHitInfo hitInfo;
			hitInfo.MonsterID = monster.second.get()->GetID();
			hitInfo.SpawnID = monster.first;
			hitInfo.Damage = 20;
			hitMonsters.push_back(hitInfo);
		}
	}

	if (!hitMonsters.empty())
	{
		const uint16_t monsterCount = static_cast<uint16_t>(hitMonsters.size());
		const uint16_t packetSize = sizeof(PacketBase)
			+ sizeof(S2CMonsterHitPacket)
			+ sizeof(S2CMonsterHitInfo) * monsterCount;

		std::shared_ptr<char[]> buffer(new char[packetSize]);
		PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.get());
		packet->PacketSize = packetSize;
		packet->PacID = S2CMonsterHit;
		S2CMonsterHitPacket* header = reinterpret_cast<S2CMonsterHitPacket*>(packet->Body);
		header->MonsterCount = monsterCount;
		S2CMonsterHitInfo* monsters = reinterpret_cast<S2CMonsterHitInfo*>(header + 1);
		memcpy(monsters, hitMonsters.data(), sizeof(S2CMonsterHitInfo) * monsterCount);

		if (!m_users.empty())
			MainServer::Instance().BroadCast(m_users, packet);
	}
}

void Map::SpawnMonster()
{
	std::vector<S2CMonsterRespawnInfo> respawnMonsters;

	for (auto& monster : m_monsters)
	{
		if (monster.second->IsDead())
		{
			// 몬스터 리스폰
			Vector3 spawnPos = { 150.0f, 150.0f, 0.0f };
			monster.second->Respawn(spawnPos);

			S2CMonsterRespawnInfo info;
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
			+ sizeof(S2CMonsterRespawnInfo) * monsterCount;

		PacketBase* packet = reinterpret_cast<PacketBase*>(new char[packetSize]);

		packet->PacketSize = packetSize;
		packet->PacID = S2CMonsterRespawn;

		S2CMonsterRespawnPacket* header = reinterpret_cast<S2CMonsterRespawnPacket*>(packet->Body);
		header->MonsterCount = monsterCount;

		S2CMonsterRespawnInfo* monsters = reinterpret_cast<S2CMonsterRespawnInfo*>(header + 1);
		memcpy(monsters, respawnMonsters.data(), sizeof(S2CMonsterRespawnInfo) * monsterCount);

		if (!m_users.empty())
			MainServer::Instance().BroadCast(m_users, packet);

		delete[] reinterpret_cast<char*>(packet);
	}
}

void Map::MonsterStateUpdate()
{
	std::vector<S2CMonsterStateInfo> monsterStates;
	monsterStates.reserve(m_monsters.size());
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
			info.MaxHP = monster.second->GetMaxHp();
			info.CurHP = monster.second->GetCurHp();
			monsterStates.push_back(info);
		}
	}

	if (!monsterStates.empty())
	{
		const uint16_t monsterCount = static_cast<uint16_t>(monsterStates.size());
		const uint16_t packetSize = sizeof(PacketBase)
			+ sizeof(S2CMonsterStatePacket)
			+ sizeof(S2CMonsterStateInfo) * monsterCount;

		std::shared_ptr<char[]> buffer(new char[packetSize]);

		PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.get());

		packet->PacketSize = packetSize;
		packet->PacID = S2CMonsterState;

		S2CMonsterStatePacket* header = reinterpret_cast<S2CMonsterStatePacket*>(packet->Body);
		header->MonsterCount = monsterCount;

		S2CMonsterStateInfo* monsters = reinterpret_cast<S2CMonsterStateInfo*>(header + 1);
		memcpy(monsters, monsterStates.data(), sizeof(S2CMonsterStateInfo) * monsterCount);

		if (!m_users.empty())
			MainServer::Instance().BroadCast(m_users, packet);
	}
}

void Map::PlayerStateUpdate()
{
	std::vector<S2CPlayerStateInfo> playerStates;
	playerStates.reserve(m_users.size());

	{
		std::scoped_lock lock(m_mutex);
		for (int userID : m_users) {
			std::shared_ptr<User> user = MainServer::Instance().GetUserByID(userID);
			if (!user)
				continue;

			S2CPlayerStateInfo info{};
			info.UserID = userID;
			strncpy_s(info.Name, sizeof(info.Name), user->GetUserName().c_str(), _TRUNCATE);
			auto pos = user->GetCharacter().GetPosition();
			info.PosX = pos.x;
			info.PosY = pos.y;
			info.PosZ = pos.z;
			info.Direction = user->GetCharacter().GetDirection();

			playerStates.push_back(info);
		}
	}

	if (playerStates.empty())
		return;

	const uint16_t playerCount = static_cast<uint16_t>(playerStates.size());
	const uint16_t packetSize = sizeof(PacketBase) + sizeof(S2CPlayerStatePacket) + sizeof(S2CPlayerStateInfo) * playerCount;

	std::shared_ptr<char[]> buffer(new char[packetSize]);
	PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.get());
	packet->PacketSize = packetSize;
	packet->PacID = S2CPlayerState;

	S2CPlayerStatePacket* header = reinterpret_cast<S2CPlayerStatePacket*>(packet->Body);
	header->PlayerCount = playerCount;

	S2CPlayerStateInfo* states = reinterpret_cast<S2CPlayerStateInfo*>(header + 1);
	memcpy(states, playerStates.data(), sizeof(S2CPlayerStateInfo) * playerCount);

	if (!m_users.empty())
		MainServer::Instance().BroadCast(m_users, packet);
}
