#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "User.h"

class AuthPacketHandler : public IPacketHandler
{
public:
    AuthPacketHandler();

    bool CanHandle(int packetID) const override;

    void Handle(std::shared_ptr<User> user, PacketBase* pac) override;

private:
    void HandleCheckID(std::shared_ptr<User> user, PacketBase* pac);
	void HandleLogin(std::shared_ptr<User> user, PacketBase* pac);
	void HandleSignUp(std::shared_ptr<User> user, PacketBase* pac);
};