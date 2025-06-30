#pragma once

#include "Packet.h"
#include "User.h"

class IPacketHandler {
public:
    virtual bool CanHandle(uint16_t packetID) const = 0;
    virtual ~IPacketHandler() = default;
};

class IAuthPacketHandler : public IPacketHandler {
public:
    virtual void Handle(unsigned int sessionID, PacketBase* pac) = 0;
};

class IUserPacketHandler : public IPacketHandler {
public:
    virtual void Handle(std::shared_ptr<User> user, PacketBase* pac) = 0;
};