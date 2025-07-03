#include "MapPacketHandler.h"
#include "MainServer.h"

MapPacketHandler::MapPacketHandler(MapManager& mapManager)
    : m_mapManager(mapManager)
{
}

bool MapPacketHandler::CanHandle(uint16_t packetID) const
{
    return packetID == C2SChangeMap || packetID == C2SPlayerAttack || packetID == C2SChangeMapByPortal;
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
    else if (packet->PacID == C2SChangeMapByPortal) 
    {
        HandleChangeMapByPortal(user, packet);
    }
}

void MapPacketHandler::ChangeUserMap(std::shared_ptr<User> user, uint16_t targetMapID)
{
    auto newMap = m_mapManager.GetMap(targetMapID);
    if (newMap != nullptr)
    {
        newMap->AddUser(user);
        user->SetCurrentMapID(targetMapID);

        std::vector<S2CChangeMapPortalInfo> portalInfos;
        portalInfos.reserve(newMap->GetPortalCount());
        for (auto& portal : newMap->GetPortals())
        {
            S2CChangeMapPortalInfo portalInfo;
            portalInfo.PortalID = portal.first;
            portalInfo.TargetMapID = portal.second.TargetMapID;
            portalInfo.PosX = portal.second.Position.x;
            portalInfo.PosY = portal.second.Position.y;
            portalInfo.PosZ = portal.second.Position.z;
            portalInfos.push_back(portalInfo);
        }

        uint16_t portalCount = static_cast<uint16_t>(portalInfos.size());
        uint16_t packetSize = sizeof(PacketBase) + sizeof(S2CChangeMapAckPacket) + sizeof(S2CChangeMapPortalInfo) * portalCount;

        std::shared_ptr<char[]> buffer(new char[packetSize]);
        PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.get());
        packet->PacketSize = packetSize;
        packet->PacID = S2CChangeMapAck;

        S2CChangeMapAckPacket* ack = reinterpret_cast<S2CChangeMapAckPacket*>(packet->Body);
        ack->Result = 1;
        ack->MapID = targetMapID;
        ack->PortalCount = portalCount;
        ack->SpawnPosX = user->GetCharacter().GetPosition().x;
        ack->SpawnPosY = user->GetCharacter().GetPosition().y;
        ack->SpawnPosZ = user->GetCharacter().GetPosition().z;

        S2CChangeMapPortalInfo* portals = reinterpret_cast<S2CChangeMapPortalInfo*>(ack + 1);
        memcpy(portals, portalInfos.data(), sizeof(S2CChangeMapPortalInfo) * portalCount);

        MainServer::Instance().SendPacket(user->GetUserID(), packet);
    }
    else 
    {
        // 실패 패킷 처리
        const uint16_t packetSize = sizeof(PacketBase) + sizeof(S2CChangeMapAckPacket);
        std::shared_ptr<char[]> buffer(new char[packetSize]);
        PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.get());
        packet->PacketSize = packetSize;
        packet->PacID = S2CChangeMapAck;

        S2CChangeMapAckPacket* ack = reinterpret_cast<S2CChangeMapAckPacket*>(packet->Body);
        ack->Result = 0;
        ack->MapID = 0;
        ack->PortalCount = 0;
        ack->SpawnPosX = 0.0f;
        ack->SpawnPosY = 0.0f;
        ack->SpawnPosZ = 0.0f;

        MainServer::Instance().SendPacket(user->GetUserID(), packet);
    }
}

void MapPacketHandler::HandleChangeMap(std::shared_ptr<User> user, PacketBase* pac)
{
    C2SChangeMapPacket changeMap{};
    memcpy(&changeMap, pac->Body, sizeof(C2SChangeMapPacket));

    unsigned int currentMapID = user->GetCurrentMapID();
    if (currentMapID != 0)
    {
        auto oldMap = m_mapManager.GetMap(currentMapID);
        if (oldMap != nullptr)
            oldMap->RemoveUser(user);
    }

    ChangeUserMap(user, changeMap.MapID);
}

void MapPacketHandler::HandlePlayerAttack(std::shared_ptr<User> user, PacketBase* packet)
{
	C2SPlayerAttackPacket attackPacket;
	memcpy(&attackPacket, packet->Body, packet->PacketSize - sizeof(PacketBase));

    unsigned int currentMapID = user->GetCurrentMapID();
    auto currentMap = m_mapManager.GetMap(currentMapID);

    if (currentMap != nullptr)
    {
        currentMap->PlayerAttack(user, attackPacket);
    }
}

void MapPacketHandler::HandleChangeMapByPortal(std::shared_ptr<User> user, PacketBase* packet)
{
    C2SChangeMapByPortalPacket changeMapByPortal;
    memcpy(&changeMapByPortal, packet->Body, packet->PacketSize - sizeof(PacketBase));
    unsigned int currentMapID = user->GetCurrentMapID();
    auto oldMap = m_mapManager.GetMap(currentMapID);
    if (oldMap != nullptr)
    {
        oldMap->RemoveUser(user);
    }

	auto portalInfo = oldMap->GetPortal(changeMapByPortal.PortalID);
    auto newMap = m_mapManager.GetMap(portalInfo.TargetMapID);
    if (newMap != nullptr)
    {
        newMap->AddUser(user);
        user->SetCurrentMapID(portalInfo.TargetMapID);

        std::vector<S2CChangeMapPortalInfo> portalInfos;
        portalInfos.reserve(newMap->GetPortalCount());
        for (auto& portal : newMap->GetPortals())
        {
            S2CChangeMapPortalInfo portalInfo;
            portalInfo.PortalID = portal.first;
            portalInfo.TargetMapID = portal.second.TargetMapID;
            portalInfo.PosX = portal.second.Position.x;
            portalInfo.PosY = portal.second.Position.y;
            portalInfo.PosZ = portal.second.Position.z;
			portalInfos.push_back(portalInfo);
        }

        if (!portalInfos.empty())
        {
            const uint16_t portalCount = static_cast<uint16_t>(portalInfos.size());
            const uint16_t packetSize = sizeof(PacketBase)
                + sizeof(S2CChangeMapAckPacket)
                + sizeof(S2CChangeMapPortalInfo) * portalCount;

            std::shared_ptr<char[]> buffer(new char[packetSize]);

            PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.get());

            packet->PacketSize = packetSize;
            packet->PacID = S2CChangeMapAck;

            S2CChangeMapAckPacket* ack = reinterpret_cast<S2CChangeMapAckPacket*>(packet->Body);
			ack->PortalCount = portalCount;
            ack->Result = 1;
            ack->MapID = portalInfo.TargetMapID;
            ack->SpawnPosX = portalInfo.SpawnPosition.x;
            ack->SpawnPosY = portalInfo.SpawnPosition.y;
            ack->SpawnPosZ = portalInfo.SpawnPosition.z;
			user->GetCharacter().Respawn(portalInfo.SpawnPosition);

            S2CChangeMapPortalInfo* portals = reinterpret_cast<S2CChangeMapPortalInfo*>(ack + 1);
            memcpy(portals, portalInfos.data(), sizeof(S2CChangeMapPortalInfo) * portalCount);

			MainServer::Instance().SendPacket(user->GetUserID(), packet);
        }
    }
    else
    {
        const uint16_t packetSize = sizeof(PacketBase)
            + sizeof(S2CChangeMapAckPacket);

        std::shared_ptr<char[]> buffer(new char[packetSize]);

        PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.get());

        packet->PacketSize = packetSize;
        packet->PacID = S2CChangeMapAck;

        S2CChangeMapAckPacket* ack = reinterpret_cast<S2CChangeMapAckPacket*>(packet->Body);
        ack->PortalCount = 0;
        ack->Result = 0;
        ack->MapID = 0;
        ack->SpawnPosX = 0;
        ack->SpawnPosY = 0;
        ack->SpawnPosZ = 0;

        MainServer::Instance().SendPacket(user->GetUserID(), packet);
    }
}
