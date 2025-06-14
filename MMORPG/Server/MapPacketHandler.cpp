#include "MapPacketHandler.h"
#include "IOCP.h"

MapPacketHandler::MapPacketHandler(std::shared_ptr<IOCP> iocp,
    MapManager& mapManager,
    std::unordered_map<unsigned int, unsigned int>& userToSessionMap)
    : m_IOCP(iocp), m_mapManager(mapManager), m_userToSessionMap(userToSessionMap)
{
}

bool MapPacketHandler::CanHandle(int packetID) const
{
    return packetID == C2SChangeMap || packetID == C2SPlayerAttack;
}

void MapPacketHandler::Handle(std::shared_ptr<User> user, PacketBase* packet)
{
    if (packet->PacID == C2SChangeMap) 
    {
        HandleChangeMap(user, packet);
    } 
    else if (packet->PacID == C2SPlayerAttack) 
    {
		HandlePlayerAttack(user, packet);
	}
}

void MapPacketHandler::HandleChangeMap(std::shared_ptr<User> user, PacketBase* packet)
{
    C2SChangeMapPacket changeMap;
    memcpy(&changeMap, packet->Body, packet->PacketSize - sizeof(PacketBase));

    unsigned int currentMapID = user.get()->GetCurrentMapID();
    auto oldMap = m_mapManager.GetMap(currentMapID);
    if (oldMap != nullptr)
    {
        oldMap->RemoveUser(user);
    }

    S2CChangeMapAckPacket ack;
    auto newMap = m_mapManager.GetMap(changeMap.MapID);
    if (newMap != nullptr)
    {
        newMap->AddUser(user);
        user.get()->SetCurrentMapID(changeMap.MapID);
        user.get()->GetCharacter().Respawn(newMap->GetUserSpawnPos());

        ack.Result = 1;
        ack.MapID = changeMap.MapID;
        ack.SpawnPosX = newMap->GetUserSpawnPos().x;
        ack.SpawnPosY = newMap->GetUserSpawnPos().y;
        ack.SpawnPosZ = 0.0f;
    }
    else
    {
        ack.Result = 0;
        ack.MapID = 0;
        ack.SpawnPosX = 0.0f;
        ack.SpawnPosY = 0.0f;
        ack.SpawnPosZ = 0.0f;
    }

    int packetSize = sizeof(PacketBase) + sizeof(S2CChangeMapAckPacket);

    std::shared_ptr<char[]> buffer(new char[packetSize]);

    PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
    newPac->PacID = S2CChangeMapAck;
    newPac->PacketSize = packetSize;
    memcpy(newPac->Body, &ack, sizeof(S2CChangeMapAckPacket));

    unsigned int sessionID = m_userToSessionMap[user.get()->GetUserID()];
    m_IOCP->SendPacket(sessionID, buffer, newPac->PacketSize);
}

void MapPacketHandler::HandlePlayerAttack(std::shared_ptr<User> user, PacketBase* packet)
{
	C2SPlayerAttackPacket attackPacket;
	memcpy(&attackPacket, packet->Body, packet->PacketSize - sizeof(PacketBase));

    unsigned int currentMapID = user.get()->GetCurrentMapID();
    auto currentMap = m_mapManager.GetMap(currentMapID);

    if (currentMap != nullptr)
    {
        currentMap->PlayerAttack(user, attackPacket);
    }
}