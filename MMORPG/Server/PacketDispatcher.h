#pragma once

#include "pch.h"
#include "IPacketHandler.h"

class User;

class PacketDispatcher
{
public:
    void RegisterHandler(std::unique_ptr<IPacketHandler> handler);
    bool DispatchPacket(std::shared_ptr<User> user, PacketBase* packet);

private:
    std::vector<std::unique_ptr<IPacketHandler>> m_handlers;
};

