#pragma once

#include "Define.h"

enum PacketID : uint16_t
{
	C2SCheckID,
	S2CCheckIDAck,
	C2SLogin,
	S2CLoginAck,
	C2SSignUp,
	S2CSignUpAck,
	C2SCheckCharacterName,
	S2CCheckCharacterNameAck,
	C2SCreateCharacter,
	S2CCreateCharacterAck,
	C2SSelectCharacter,
	S2CSelectCharacterAck,
	C2SRanking,
	S2CRankingAck,

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
	C2SChangeMapByPortal,

	// 몬스터 관련
	S2CMonsterRespawn = 400,
	S2CMonsterState,
	S2CMonsterHit,

	// 플레이어 관련
	C2SPlayerMove = 500,
	S2CPlayerMove,
	C2SPlayerStop,
	S2CPlayerPosSync,
	C2SPlayerAttack,
	S2CPlayerAttack,
	C2SPlayerChat,
	S2CPlayerChat,
	S2CExpGain
};

#pragma pack(push, 1)
struct PacketBase
{
	uint16_t PacketSize;
	PacketID PacID;
	char Body[];
};

struct C2SConnectPacket
{
};

struct S2CConnectAckPacket
{
	uint8_t Result; // 0: 성공, 1: 실패
};

struct C2SDisconnectPacket
{
};

struct C2SCheckIDPacket
{
	char ID[ID_SIZE];
};

struct S2CCheckIDAckPacket
{
	uint8_t Result; // 0: 사용 가능, 1: 사용 불가
};

struct C2SLoginPacket
{
	char ID[ID_SIZE];
	char Password[PASSWORD_SIZE];
};

struct S2CLoginAckPacket
{
	uint8_t Result; // 0: 성공, 1: 실패
	uint16_t UserID; // 로그인 성공 시 사용자 ID 반환
	char ID[ID_SIZE]; // 로그인 성공 시 사용자 이름 반환
	uint16_t CharacterCount;
};

struct S2CCharacterInfo
{
	uint16_t CharacterID;
	char Name[NAME_SIZE];
	uint8_t Gender; // 0: 남성, 1: 여성
	uint16_t Level;
	uint32_t Exp;
};

struct C2SSignUpPacket
{
	char ID[ID_SIZE];
	char Password[PASSWORD_SIZE];
};

struct S2CSignUpAckPacket
{
	uint8_t Result; // 0: 성공, 1: 실패
};

struct C2SCheckCharacterNamePacket
{
	char Name[NAME_SIZE];
};

struct S2CCheckCharacterNameAckPacket
{
	uint8_t Result; // 0: 사용 가능, 1: 사용 불가
};

struct C2SCreateCharacterPacket
{
	char Name[NAME_SIZE];
	uint8_t Gender; // 0: 남성, 1: 여성
};

struct S2CCreateCharacterAckPacket
{
	uint8_t Result; // 0: 성공, 1: 실패
	uint16_t CharacterID;
	char Name[NAME_SIZE];
	uint8_t Gender; // 0: 남성, 1: 여성
};

struct C2SSelectCharacterPacket
{
	uint16_t CharacterID;
};

struct S2CSelectCharacterAckPacket
{
	uint8_t Result; // 0: 성공, 1: 실패
	uint16_t CharacterID;
	char Name[NAME_SIZE];
	uint16_t Level;
	uint32_t Exp;
	float PosX;
	float PosY;
	float PosZ;
	uint16_t MapID;
};

struct C2SRankingPacket
{
};

struct S2CRankingInfo
{
	uint16_t CharacterID;
	char Name[20];
	uint16_t Level;
};

struct S2CRankingAckPacket
{
	uint8_t Count;
	uint16_t MyRank;
	S2CRankingInfo Rankers[10];
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
	uint16_t PortalCount;
};

struct S2CChangeMapPortalInfo
{
	uint16_t PortalID;
	float PosX;
	float PosY;
	float PosZ;
	uint16_t TargetMapID;
};

struct C2SChangeMapByPortalPacket
{
	uint16_t PortalID;
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

	uint8_t Direction;
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
	uint8_t MoveDirection; // 0: 상, 1: 하, 2: 좌, 3: 우
	uint32_t FrameID;
};

struct S2CPlayerMovePacket
{
	uint16_t UserID;
	uint8_t MoveDirection;
};

struct C2SPlayerStopPacket
{
	uint32_t FrameID;
};

struct S2CPlayerPosSyncPacket
{
	float PosX;
	float PosY;
	float PosZ;
	uint32_t AckFrameID;
};;

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

struct S2CExpGainPacket
{
	uint32_t ExpGained;
	uint32_t TotalExp;
	uint16_t Level;
};
#pragma pack(pop)