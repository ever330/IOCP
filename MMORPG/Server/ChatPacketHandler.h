#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "User.h"
#include "Map.h"

class IOCP;

class ChatPacketHandler : public IPacketHandler
{
public:
    ChatPacketHandler(std::shared_ptr<IOCP> iocp, std::unordered_map<unsigned int, std::shared_ptr<Map>>& maps);

    bool CanHandle(int packetID) const override;

    void Handle(std::shared_ptr<User> user, PacketBase* packet) override;

private:
    std::shared_ptr<IOCP> m_IOCP;
    std::unordered_map<unsigned int, std::shared_ptr<Map>>& m_maps;
};