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
    // DB부분 추가되면 해당 부분은 Login 관련 처리로 변경될 예정
	void HandleSetName(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerMove(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerStop(std::shared_ptr<User> user, PacketBase* pac);
	void HandleCheckName(std::shared_ptr<User> user, PacketBase* pac);
	void HandleCreateCharacter(std::shared_ptr<User> user, PacketBase* pac);
	void HandleSelectCharacter(std::shared_ptr<User> user, PacketBase* pac);
	void HandleRanking(std::shared_ptr<User> user, PacketBase* pac);
	void HandlePlayerPosSync(std::shared_ptr<User> user, PacketBase* pac);

    // 임시 유저 ID 생성용
	unsigned int m_userID = 1;
};

