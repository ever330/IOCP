#pragma once

#include "Packet.h"
#include "User.h"

class IPacketHandler
{
public:
    virtual void Handle(std::shared_ptr<User> user, PacketBase* pac) = 0;
    virtual bool CanHandle(int packetID) const = 0;
    virtual ~IPacketHandler() = default;
};