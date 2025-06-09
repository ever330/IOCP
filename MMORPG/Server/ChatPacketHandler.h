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
    ChatPacketHandler(std::shared_ptr<IOCP> iocp, MapManager& mapManager);

    bool CanHandle(int packetID) const override;

    void Handle(std::shared_ptr<User> user, PacketBase* packet) override;

private:
    std::shared_ptr<IOCP> m_IOCP;
	MapManager& m_mapManager;
};