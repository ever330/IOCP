#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "User.h"

class AuthPacketHandler : public IAuthPacketHandler
{
public:
    AuthPacketHandler();

    bool CanHandle(uint16_t packetID) const override;

    void Handle(unsigned int sessionID, PacketBase* pac) override;

private:
    void HandleCheckID(unsigned int sessionID, PacketBase* pac);
	void HandleLogin(unsigned int sessionID, PacketBase* pac);
	void HandleSignUp(unsigned int sessionID, PacketBase* pac);
};