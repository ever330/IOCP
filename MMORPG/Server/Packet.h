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

	// 몬스터 관련
	S2CMonsterRespawn = 400,
	S2CMonsterState,

	// 플레이어 관련
	C2SPlayerMove = 500,
	S2CPlayerMove,
	C2SPlayerAction,
	S2CPlayerAction,
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
	char Name[NAME_SIZE];
};

struct S2CMonsterRespawnPacket
{
	uint16_t MonsterCount;
};

struct MonsterRespawnInfo
{
	uint16_t MonsterID;	// 실제 DB를 참고하게 될 ID
	uint16_t SpawnID;	// 각 맵에서 부여받는 ID

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

	uint8_t Direction;		// 0 : 왼쪽, 1 : 오른쪽

	uint16_t CurHP;
};

struct C2SPlayerMovePacket
{
	float PosX;
	float PosY;
	float PosZ;

	float DirX;
	float DirY;
	float DirZ;

	uint8_t MoveType;		// 걷기, 달리기, 점프 등
};

struct S2CPlayerMovePacket
{
	uint16_t UserID;

	float PosX;
	float PosY;
	float PosZ;

	float DirX;
	float DirY;
	float DirZ;

	uint8_t MoveType;
};

struct C2SPlayerActionPacket
{
	uint16_t ActionType;	// 공격, 스킬 등
	uint32_t TargetID;      // 몬스터 ID, 유저 ID 등
	float DirX, DirY, DirZ; // 바라보는 방향
};

struct S2CPlayerActionPacket
{
	uint16_t UserID;
	uint16_t ActionType;	
	uint32_t TargetID;
	uint32_t Damage;
	float DirX, DirY, DirZ; 
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