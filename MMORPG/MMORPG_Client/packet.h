#pragma once

#include "Define.h"

enum PacketID : uint16_t
{
	// 기존 임시용
	C2SSetName = 100,
	S2CNewUserAlert,
	C2SSendMSG,
	S2CSendMSG,

	// 하트비트
	C2SHeartBeat = 200,
	S2CHeartBeat,

	// 맵 관련
	C2SChangeMap = 300,
	S2CChangeMapAck,
	S2CPlayerEnter,
	S2CPlayerLeave,
	S2CPlayerState,

	// 몬스터 관련
	S2CMonsterRespawn = 400,
	S2CMonsterState,
	S2CMonsterHit,

	// 플레이어 관련
	C2SPlayerMove = 500,
	S2CPlayerMove,
	C2SPlayerAttack,
	S2CPlayerAttack,
	C2SPlayerChat,
	S2CPlayerChat,
};

#pragma pack(push, 1)
struct PacketBase
{
	uint16_t PacketSize;
	PacketID PacID;
	char Body[];
};

struct C2SSetNamePacket
{
	char Name[NAME_SIZE];
};

struct S2CNewUserAlertPacket
{
	char Name[NAME_SIZE];
};

struct C2SSendMSGPacket
{
	char Name[NAME_SIZE];
	char MSG[MSG_SIZE];
};

struct S2CSendMSGPacket
{
	char Name[NAME_SIZE];
	char MSG[MSG_SIZE];
};

struct C2SChangeMapPacket
{
	uint16_t MapID;
};

struct S2CChangeMapAckPacket
{
	uint8_t Result;
	uint16_t MapID;
	float SpawnPosX;
	float SpawnPosY;
	float SpawnPosZ;
};

struct S2CPlayerEnterPacket
{
	uint16_t UserID;
	char Name[NAME_SIZE];
	float SpawnPosX;
	float SpawnPosY;
	float SpawnPosZ;
};

struct S2CPlayerLeavePacket
{
	uint16_t UserID;
	char Name[NAME_SIZE];
};

struct S2CPlayerStatePacket
{
	uint16_t PlayerCount;
};

struct S2CPlayerStateInfo
{
	uint16_t UserID;
	char Name[NAME_SIZE];

	float PosX;
	float PosY;
	float PosZ;
};

struct S2CMonsterRespawnPacket
{
	uint16_t MonsterCount;
};

struct S2CMonsterRespawnInfo
{
	uint16_t MonsterID;
	uint16_t SpawnID;

	float SpawnPosX;
	float SpawnPosY;
	float SpawnPosZ;

	uint16_t MaxHP;
	uint16_t CurHP;
};

struct S2CMonsterStatePacket
{
	uint16_t MonsterCount;
};

struct S2CMonsterStateInfo
{
	uint16_t MonsterID;
	uint16_t SpawnID;

	float PosX;
	float PosY;
	float PosZ;

	uint16_t MaxHP;
	uint16_t CurHP;
};

struct C2SPlayerMovePacket
{
	uint8_t MoveDirection; // 0: LEFT, 1: RIGHT, 2: UP, 3: DOWN
};

struct S2CPlayerMovePacket
{
	char Name[NAME_SIZE];
	uint8_t MoveDirection;
};

struct C2SPlayerAttackPacket
{
	uint8_t AttackDirection; 
};

struct S2CPlayerAttackPacket
{
	uint16_t UserID;
	uint8_t AttackDirection;
};

struct S2CMonsterHitPacket
{
	uint16_t MonsterCount;
};

struct S2CMonsterHitInfo
{
	uint16_t MonsterID;
	uint16_t SpawnID;

	uint8_t AttackDirection;

	uint16_t Damage;
};

struct C2SPlayerChatPacket
{
	char ChatMsg[MSG_SIZE];
};

struct S2CPlayerChatPacket
{
	uint16_t UserID;
	char Name[NAME_SIZE];
	char ChatMsg[MSG_SIZE];
};
#pragma pack(pop)