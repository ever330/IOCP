#pragma once

#include "Packet.h"
#include "User.h"

class IPacketHandler
{
public:
    virtual void Handle(std::shared_ptr<User> user, PacketBase* pac) = 0;
    // 유저 연결, 해제 시 호출
    virtual void Handle(unsigned int sessionID, PacketBase* pac) {};
    virtual bool CanHandle(int packetID) const = 0;
    virtual ~IPacketHandler() = default;
};