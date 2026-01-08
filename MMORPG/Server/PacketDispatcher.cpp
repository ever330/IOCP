#include "pch.h"
#include "PacketDispatcher.h"
#include "User.h"

void PacketDispatcher::RegisterAuthHandler(std::unique_ptr<IAuthPacketHandler> handler)
{
    m_authHandlers.push_back(std::move(handler));
}

void PacketDispatcher::RegisterUserHandler(std::unique_ptr<IUserPacketHandler> handler)
{
	m_userHandlers.push_back(std::move(handler));
}

void PacketDispatcher::RegisterMapHandler(std::unique_ptr<MapPacketHandler> handler)
{
    m_mapPacketHandler = std::move(handler);
}

bool PacketDispatcher::DispatchPacket(unsigned int sessionID, PacketBase* packet)
{
    for (const auto& handler : m_authHandlers) 
    {
        if (handler->CanHandle(packet->PacID)) 
        {
            handler->Handle(sessionID, packet);
            return true;
        }
    }
    return false;
}

MapPacketHandler* PacketDispatcher::GetMapPacketHandler()
{
    return m_mapPacketHandler.get();
}

bool PacketDispatcher::DispatchPacket(std::shared_ptr<User> user, PacketBase* packet)
{
    for (const auto& handler : m_userHandlers) 
    {
        if (handler->CanHandle(packet->PacID)) 
        {
            handler->Handle(user, packet);
            return true;
        }
    }
    if (m_mapPacketHandler->CanHandle(packet->PacID))
    {
        m_mapPacketHandler->Handle(user, packet);
        return true;
    }
    return false;
}