#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "User.h"

class IOCP;

class UserPacketHandler : public IPacketHandler 
{
public:
    UserPacketHandler(std::shared_ptr<IOCP> iocp);

    bool CanHandle(int packetID) const override;

    void Handle(std::shared_ptr<User> user, PacketBase* pac) override;

private:
	void HandleSetName(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerMove(std::shared_ptr<User> user, PacketBase* pac);

private:
    std::shared_ptr<IOCP> m_IOCP;
    std::mutex m_mutex;
};

