#pragma once

#include "pch.h"
#include "IPacketHandler.h"

class User;

class PacketDispatcher
{
public:
    void RegisterAuthHandler(std::unique_ptr<IAuthPacketHandler> handler);
    void RegisterUserHandler(std::unique_ptr<IUserPacketHandler> handler);
    bool DispatchPacket(std::shared_ptr<User> user, PacketBase* packet);
    bool DispatchPacket(unsigned int sessionID, PacketBase* packet);

private:
    std::vector<std::unique_ptr<IAuthPacketHandler>> m_authHandlers;
    std::vector<std::unique_ptr<IUserPacketHandler>> m_userHandlers;
};

