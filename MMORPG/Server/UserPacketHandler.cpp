#include "UserPacketHandler.h"
#include "IOCP.h"
#include "User.h"
#include "MainServer.h"

UserPacketHandler::UserPacketHandler(std::shared_ptr<IOCP> iocp)
	: m_IOCP(iocp)
{
}

bool UserPacketHandler::CanHandle(int packetID) const
{
	return packetID == C2SSetName || packetID == C2SPlayerMove;
}

void UserPacketHandler::Handle(std::shared_ptr<User> user, PacketBase* pac)
{
	if (pac->PacID == C2SConnect)
	{
		HandleUserConnect(user, pac);
	}
	else if (pac->PacID == C2SSetName)
	{
		HandleSetName(user, pac);
	}
	else if (pac->PacID == C2SPlayerMove)
	{
		HandlePlayerMove(user, pac);
	}
}

void UserPacketHandler::HandleUserConnect(std::shared_ptr<User> dummyUser, PacketBase* pac)
{
	auto userId = m_userID;
	auto user = std::make_shared<User>(userId, "");

	m_userID++;

	MainServer::Instance().AddUser(user);
}

void UserPacketHandler::HandleUserDisconnect(std::shared_ptr<User> user, PacketBase* pac)
{
	MainServer::Instance().DisconnectUser(user.get()->GetUserID());
}

void UserPacketHandler::HandleSetName(std::shared_ptr<User> user, PacketBase* pac)
{
	C2SSetNamePacket setName;
	memcpy(&setName, pac->Body, sizeof(setName));

	user->SetUserName(setName.Name);

	user->SetConnected(true);

	int packetSize = pac->PacketSize;

	std::shared_ptr<char[]> buffer(new char[packetSize]);
	PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
	newPac->PacID = S2CNewUserAlert;
	newPac->PacketSize = packetSize;
	memcpy(newPac->Body, &setName, packetSize - sizeof(PacketBase));

	//m_IOCP->BroadCast(buffer, packetSize);
}

void UserPacketHandler::HandlePlayerMove(std::shared_ptr<User> user, PacketBase* pac)
{
	C2SPlayerMovePacket move;
	memcpy(&move, pac->Body, sizeof(move));

	user->GetCharacter().Move(move.MoveDirection);

	// 이동 브로드캐스트
	S2CPlayerMovePacket response{};
	memcpy(response.Name, user->GetUserName().c_str(), sizeof(response.Name));
	response.MoveDirection = move.MoveDirection;

	int packetSize = sizeof(PacketBase) + sizeof(response);
	std::shared_ptr<char[]> buffer(new char[packetSize]);
	PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
	newPac->PacID = S2CPlayerMove;
	newPac->PacketSize = packetSize;
	memcpy(newPac->Body, &response, sizeof(response));

	//m_IOCP->BroadCast(buffer, packetSize);
}