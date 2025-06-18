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
    void HandleUserConnect(std::shared_ptr<User> dummyUser, PacketBase* pac);
	void HandleUserDisconnect(std::shared_ptr<User> user, PacketBase* pac);
    // DB부분 추가되면 해당 부분은 Login 관련 처리로 변경될 예정
	void HandleSetName(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerMove(std::shared_ptr<User> user, PacketBase* pac);

private:
    std::shared_ptr<IOCP> m_IOCP;
    std::mutex m_mutex;

    // 임시 유저 ID 생성용
	unsigned int m_userID = 1;
};

