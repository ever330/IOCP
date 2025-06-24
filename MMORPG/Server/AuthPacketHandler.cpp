#include "AuthPacketHandler.h"
#include "MainServer.h"

AuthPacketHandler::AuthPacketHandler()
{
}

bool AuthPacketHandler::CanHandle(int packetID) const
{
	return packetID == C2SCheckID || packetID == C2SLogin || packetID == C2SSignUp;
}

void AuthPacketHandler::Handle(std::shared_ptr<User> user, PacketBase* pac)
{
	if (pac->PacID == C2SCheckID)
	{
		HandleCheckID(user, pac);
	}
	else if (pac->PacID == C2SLogin)
	{
		HandleLogin(user, pac);
	}
	else if (pac->PacID == C2SSignUp)
	{
		HandleSignUp(user, pac);
	}
}

void AuthPacketHandler::HandleCheckID(std::shared_ptr<User> user, PacketBase* pac)
{
	C2SCheckIDPacket checkPacket;
	memcpy(&checkPacket, pac->Body, pac->PacketSize - sizeof(PacketBase));

	std::string username = EscapeSQL(std::string(checkPacket.ID));
	std::string query = "SELECT COUNT(*) AS cnt FROM Users WHERE Username = '" + username + "';";

	MainServer::Instance().RequestQuery(query, [user](bool success, sql::ResultSet* res) {
		S2CCheckIDAckPacket ack;
		ack.Result = 1; // 기본은 사용 불가로 설정

		if (success && res && res->next()) 
		{
			int count = res->getInt("cnt");
			ack.Result = (count == 0) ? 0 : 1;
		}

		delete res;

		int packetSize = sizeof(PacketBase) + sizeof(S2CCheckIDAckPacket);

		std::shared_ptr<char[]> buffer(new char[packetSize]);

		PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
		newPac->PacID = S2CCheckIDAck;
		newPac->PacketSize = packetSize;
		memcpy(newPac->Body, &ack, sizeof(S2CCheckIDAckPacket));


		MainServer::Instance().SendPacket(user->GetUserID(), newPac);
		});
}

void AuthPacketHandler::HandleLogin(std::shared_ptr<User> user, PacketBase* pac)
{
}

void AuthPacketHandler::HandleSignUp(std::shared_ptr<User> user, PacketBase* pac)
{
}
