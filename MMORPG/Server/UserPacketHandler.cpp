#include "UserPacketHandler.h"
#include "User.h"
#include "MainServer.h"

UserPacketHandler::UserPacketHandler()
{
}

bool UserPacketHandler::CanHandle(uint16_t packetID) const
{
	return packetID == C2SSetName || packetID == C2SPlayerMove || packetID == C2SPlayerStop;
}

void UserPacketHandler::Handle(std::shared_ptr<User> user, PacketBase* pac)
{
	if (pac->PacID == C2SSetName)
	{
		HandleSetName(user, pac);
	}
	else if (pac->PacID == C2SPlayerMove)
	{
		HandlePlayerMove(user, pac);
	}
	else if (pac->PacID == C2SPlayerStop)
	{
		HandlePlayerStop(user, pac);
	}
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
}

void UserPacketHandler::HandlePlayerMove(std::shared_ptr<User> user, PacketBase* pac)
{
    C2SPlayerMovePacket move;
    memcpy(&move, pac->Body, sizeof(move));

	if (move.FrameID < user->GetLastInputFrame())
		return; // ���� �Է� ����

    user->GetCharacter().SetDirection(move.MoveDirection);
    user->GetCharacter().SetMoving(true);
    user->SetLastInputFrame(move.FrameID); // Ŭ�� ���� frameID ����

    S2CPlayerMovePacket movePacket{};
    movePacket.UserID = user->GetUserID();
    movePacket.MoveDirection = move.MoveDirection;

    int packetSize = sizeof(PacketBase) + sizeof(movePacket);
    std::shared_ptr<char[]> buffer(new char[packetSize]);
    PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
    newPac->PacID = S2CPlayerMove;
    newPac->PacketSize = packetSize;
    memcpy(newPac->Body, &movePacket, sizeof(movePacket));

    // ���� �� �����鿡�� ���� ��ε�ĳ��Ʈ
    auto map = MainServer::Instance().GetMapManager().GetMap(user->GetCurrentMapID());

    if (map)
        MainServer::Instance().BroadCast(map->GetUsers(), newPac);


    S2CPlayerPosSyncPacket sync{};
	Vector3 pos = user->GetCharacter().GetPosition();
    sync.PosX = pos.x;
	sync.PosY = pos.y;
	sync.PosZ = pos.z;
    sync.AckFrameID = move.FrameID; // ���������� ó���� Ŭ�� �Է�

    int packetSize2 = sizeof(PacketBase) + sizeof(sync);
    std::shared_ptr<char[]> buffer2(new char[packetSize2]);
    PacketBase* response = reinterpret_cast<PacketBase*>(buffer2.get());
    response->PacID = S2CPlayerPosSync;
    response->PacketSize = packetSize2;
    memcpy(response->Body, &sync, sizeof(sync));

	MainServer::Instance().SendPacket(user->GetUserID(), response); // �������� ���� ����
}

void UserPacketHandler::HandlePlayerStop(std::shared_ptr<User> user, PacketBase* pac)
{
	C2SPlayerStopPacket stop;
	memcpy(&stop, pac->Body, sizeof(stop));

	if (stop.FrameID < user->GetLastInputFrame())
		return; // ���� �Է� ����

	// �÷��̾ ������ �� ó�� (��: �̵� ���� �÷��� ����)
	user->GetCharacter().SetMoving(false);
	user->SetLastInputFrame(stop.FrameID); // Ŭ�� ���� frameID ����
}