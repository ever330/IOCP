#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "User.h"

class IOCP;

class UserPacketHandler : public IPacketHandler 
{
public:
    UserPacketHandler();

    bool CanHandle(int packetID) const override;

    void Handle(std::shared_ptr<User> user, PacketBase* pac) override;

	void Handle(unsigned int sessionID, PacketBase* pac) override;

private:
    void HandleUserConnect(unsigned int sessionID, PacketBase* pac);
	void HandleUserDisconnect(std::shared_ptr<User> user, PacketBase* pac);
    // DB부분 추가되면 해당 부분은 Login 관련 처리로 변경될 예정
	void HandleSetName(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerMove(std::shared_ptr<User> user, PacketBase* pac);

    // 임시 유저 ID 생성용
	unsigned int m_userID = 1;
};

