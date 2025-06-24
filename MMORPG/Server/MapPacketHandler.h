#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "User.h"
#include "Map.h"
#include "MapManager.h"

class IOCP;

class MapPacketHandler : public IPacketHandler
{
public:
    MapPacketHandler(MapManager& mapManager,
        std::unordered_map<unsigned int, unsigned int>& userToSessionMap);

    bool CanHandle(int packetID) const override;

    void Handle(std::shared_ptr<User> user, PacketBase* pac) override;

private:
	void HandleChangeMap(std::shared_ptr<User> user, PacketBase* packet);
	void HandlePlayerAttack(std::shared_ptr<User> user, PacketBase* packet);

	MapManager& m_mapManager;
    std::unordered_map<unsigned int, unsigned int>& m_userToSessionMap;
};