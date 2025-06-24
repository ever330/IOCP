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
    // DB�κ� �߰��Ǹ� �ش� �κ��� Login ���� ó���� ����� ����
	void HandleSetName(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerMove(std::shared_ptr<User> user, PacketBase* pac);

    // �ӽ� ���� ID ������
	unsigned int m_userID = 1;
};

