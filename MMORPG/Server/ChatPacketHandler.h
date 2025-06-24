#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "User.h"
#include "Map.h"
#include "MapManager.h"

class IOCP;

class ChatPacketHandler : public IPacketHandler
{
public:
    ChatPacketHandler(MapManager& mapManager);

    bool CanHandle(int packetID) const override;

    void Handle(std::shared_ptr<User> user, PacketBase* pac) override;

private:
	MapManager& m_mapManager;
};