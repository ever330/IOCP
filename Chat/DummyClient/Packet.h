#pragma once

#include "Define.h"

enum PacketID : uint16_t
{
	C2SSetName = 100,
	S2CNewUserAlert,
	C2SSendMSG,
	S2CSendMSG
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