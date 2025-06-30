#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "User.h"
#include "Map.h"
#include "MapManager.h"

class MapPacketHandler : public IUserPacketHandler
{
public:
    MapPacketHandler(MapManager& mapManager);

    bool CanHandle(uint16_t packetID) const override;

    void Handle(std::shared_ptr<User> user, PacketBase* pac) override;

private:
	void HandleChangeMap(std::shared_ptr<User> user, PacketBase* packet);
	void HandlePlayerAttack(std::shared_ptr<User> user, PacketBase* packet);

	MapManager& m_mapManager;
};