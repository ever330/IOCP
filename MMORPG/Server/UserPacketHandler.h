#pragma once

#include "pch.h"
#include "IPacketHandler.h"
#include "User.h"

class UserPacketHandler : public IUserPacketHandler
{
public:
    UserPacketHandler();

    bool CanHandle(uint16_t packetID) const override;

    void Handle(std::shared_ptr<User> user, PacketBase* pac) override;

private:
    // DB�κ� �߰��Ǹ� �ش� �κ��� Login ���� ó���� ����� ����
	void HandleSetName(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerMove(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerStop(std::shared_ptr<User> user, PacketBase* pac);
	void HandleCheckName(std::shared_ptr<User> user, PacketBase* pac);
	void HandleCreateCharacter(std::shared_ptr<User> user, PacketBase* pac);
	void HandleSelectCharacter(std::shared_ptr<User> user, PacketBase* pac);
	void HandleRanking(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerPosSync(std::shared_ptr<User> user, PacketBase* pac);

    // �ӽ� ���� ID ������
	unsigned int m_userID = 1;
};

