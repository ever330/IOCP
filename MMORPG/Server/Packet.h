#pragma once

#include "Define.h"

enum PacketID : uint16_t
{
	// ���� �ӽÿ�
	C2SSetName = 100,
	S2CNewUserAlert,
	C2SSendMSG,
	S2CSendMSG,

	// ��Ʈ��Ʈ
	C2SHeartBeat = 200,
	S2CHeartBeat,

	// �� ����
	C2SChangeMap = 300,
	S2CChangeMapAck,
	S2CPlayerEnter,
	S2CPlayerLeave,

	// ���� ����
	S2CMonsterRespawn = 400,
	S2CMonsterState,

	// �÷��̾� ����
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
	uint16_t MonsterID;	// ���� DB�� �����ϰ� �� ID
	uint16_t SpawnID;	// �� �ʿ��� �ο��޴� ID

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

	uint8_t Direction;		// 0 : ����, 1 : ������

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

	uint8_t MoveType;		// �ȱ�, �޸���, ���� ��
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
	uint16_t ActionType;	// ����, ��ų ��
	uint32_t TargetID;      // ���� ID, ���� ID ��
	float DirX, DirY, DirZ; // �ٶ󺸴� ����
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