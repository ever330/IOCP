#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "MapPacketHandler.h"

class User;

class PacketDispatcher
{
public:
    void RegisterAuthHandler(std::unique_ptr<IAuthPacketHandler> handler);
    void RegisterUserHandler(std::unique_ptr<IUserPacketHandler> handler);
    void RegisterMapHandler(std::unique_ptr<MapPacketHandler> handler);
    bool DispatchPacket(std::shared_ptr<User> user, PacketBase* packet);
    bool DispatchPacket(unsigned int sessionID, PacketBase* packet);

    MapPacketHandler* GetMapPacketHandler();

private:
    std::vector<std::unique_ptr<IAuthPacketHandler>> m_authHandlers;
    std::vector<std::unique_ptr<IUserPacketHandler>> m_userHandlers;
    // �� �̵� ��Ŷ �ڵ鷯�� ������ ����
	std::unique_ptr<MapPacketHandler> m_mapPacketHandler;
};

