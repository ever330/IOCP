#include "AuthPacketHandler.h"
#include "MainServer.h"

AuthPacketHandler::AuthPacketHandler()
{
}

bool AuthPacketHandler::CanHandle(uint16_t packetID) const
{
	return packetID == C2SCheckID || packetID == C2SLogin || packetID == C2SSignUp;
}

void AuthPacketHandler::Handle(unsigned int sessionID, PacketBase* pac)
{
	if (pac->PacID == C2SCheckID)
	{
		HandleCheckID(sessionID, pac);
	}
	else if (pac->PacID == C2SLogin)
	{
		HandleLogin(sessionID, pac);
	}
	else if (pac->PacID == C2SSignUp)
	{
		HandleSignUp(sessionID, pac);
	}
}

void AuthPacketHandler::HandleCheckID(unsigned int sessionID, PacketBase* pac)
{
	C2SCheckIDPacket checkPacket;
	memcpy(&checkPacket, pac->Body, pac->PacketSize - sizeof(PacketBase));

	std::string username = EscapeSQL(std::string(checkPacket.ID));
	std::string query = "SELECT COUNT(*) AS cnt FROM Users WHERE Username = '" + username + "';";

	MainServer::Instance().RequestQuery(query, [sessionID](bool success, sql::ResultSet* res) {
		S2CCheckIDAckPacket ack;
		ack.Result = 1;

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

		MainServer::Instance().SendPacketBySessionID(sessionID, newPac);
		});
}

void AuthPacketHandler::HandleLogin(unsigned int sessionID, PacketBase* pac)
{
	C2SLoginPacket loginPacket;
	memcpy(&loginPacket, pac->Body, pac->PacketSize - sizeof(PacketBase));

	std::string username = EscapeSQL(std::string(loginPacket.ID));
	std::string passwordHash = Sha256(std::string(loginPacket.Password));

	std::string query =
		"SELECT UserID, Username FROM Users WHERE Username = '" + username +
		"' AND PasswordHash = '" + passwordHash + "' AND IsBanned = 0;";

	MainServer::Instance().RequestQuery(query, [sessionID](bool success, sql::ResultSet* res)
		{
			S2CLoginAckPacket ack{};
			ack.Result = 1;

			if (!success || !res || !res->next())
			{
				delete res;

				// �α��� ���� ��Ŷ ����
				int packetSize = sizeof(PacketBase) + sizeof(S2CLoginAckPacket);
				std::shared_ptr<char[]> buffer(new char[packetSize]);
				PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
				newPac->PacID = S2CLoginAck;
				newPac->PacketSize = packetSize;
				memcpy(newPac->Body, &ack, sizeof(S2CLoginAckPacket));
				MainServer::Instance().SendPacketBySessionID(sessionID, newPac);
				return;
			}

			// �α��� ���� ó��
			ack.Result = 0;
			ack.UserID = res->getInt("UserID");
			std::string name = res->getString("Username");
			memset(ack.ID, 0, ID_SIZE);
			memcpy(ack.ID, name.c_str(), min(name.size(), static_cast<size_t>(ID_SIZE - 1)));

			auto user = std::make_shared<User>(ack.UserID, name);
			MainServer::Instance().AddUser(user);
			MainServer::Instance().BindSession(sessionID, user);

			delete res;

			// ĳ���� ��� ����
			std::string charQuery =
				"SELECT CharID, Name, Level, Exp, Gender FROM Characters WHERE UserID = " + std::to_string(ack.UserID) + ";";

			MainServer::Instance().RequestQuery(charQuery, [sessionID, ack](bool success2, sql::ResultSet* charRes) mutable
				{
					std::vector<S2CCharacterInfo> characters;

					if (success2 && charRes)
					{
						while (charRes->next())
						{
							S2CCharacterInfo info{};
							info.CharacterID = charRes->getInt("CharID");

							std::string charName = charRes->getString("Name");
							memset(info.Name, 0, NAME_SIZE);
							memcpy(info.Name, charName.c_str(), min(charName.size(), static_cast<size_t>(NAME_SIZE - 1)));

							info.Level = charRes->getInt("Level");
							info.Exp = charRes->getInt("Exp");
							info.Gender = static_cast<uint8_t>(charRes->getInt("Gender"));

							characters.push_back(info);
						}
					}

					delete charRes;

					// ĳ���� �� �ݿ�
					ack.CharacterCount = static_cast<uint16_t>(characters.size());

					// ��ü ��Ŷ ũ�� = PacketBase + LoginAck + characterInfo * N
					int packetSize = sizeof(PacketBase)
						+ sizeof(S2CLoginAckPacket)
						+ sizeof(S2CCharacterInfo) * characters.size();

					std::shared_ptr<char[]> buffer(new char[packetSize]);

					PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
					newPac->PacID = S2CLoginAck;
					newPac->PacketSize = packetSize;

					char* writePtr = newPac->Body;

					// 1. �α��� ���� ��Ŷ
					memcpy(writePtr, &ack, sizeof(S2CLoginAckPacket));
					writePtr += sizeof(S2CLoginAckPacket);

					// 2. ĳ���� ������ ���̱�
					if (!characters.empty())
						memcpy(writePtr, characters.data(), sizeof(S2CCharacterInfo) * characters.size());

					MainServer::Instance().SendPacketBySessionID(sessionID, newPac);
				});
		});
}

void AuthPacketHandler::HandleSignUp(unsigned int sessionID, PacketBase* pac)
{
	C2SSignUpPacket signUpPacket;
	memcpy(&signUpPacket, pac->Body, pac->PacketSize - sizeof(PacketBase));

	std::string username = EscapeSQL(std::string(signUpPacket.ID));
	std::string passwordHash = Sha256(std::string(signUpPacket.Password));

	std::string checkQuery = "SELECT COUNT(*) AS cnt FROM Users WHERE Username = '" + username + "';";
	MainServer::Instance().RequestQuery(checkQuery, [username, passwordHash, sessionID](bool success, sql::ResultSet* res) {
		bool canRegister = false;

		if (success && res && res->next())
		{
			int count = res->getInt("cnt");
			canRegister = (count == 0);
		}

		delete res;

		if (!canRegister)
		{
			S2CSignUpAckPacket ack{ 1 };
			int packetSize = sizeof(PacketBase) + sizeof(S2CSignUpAckPacket);
			std::shared_ptr<char[]> buffer(new char[packetSize]);

			PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
			newPac->PacID = S2CSignUpAck;
			newPac->PacketSize = packetSize;
			memcpy(newPac->Body, &ack, sizeof(S2CSignUpAckPacket));

			MainServer::Instance().SendPacketBySessionID(sessionID, newPac);
			return;
		}

		std::string insertQuery = "INSERT INTO Users (Username, PasswordHash) "
			"VALUES ('" + username + "', '" + passwordHash + "');";

		MainServer::Instance().RequestQuery(insertQuery, [sessionID](bool insertSuccess, sql::ResultSet* dummyRes) {
			delete dummyRes;

			S2CSignUpAckPacket ack{ insertSuccess ? 0 : 1 };

			int packetSize = sizeof(PacketBase) + sizeof(S2CSignUpAckPacket);
			std::shared_ptr<char[]> buffer(new char[packetSize]);

			PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
			newPac->PacID = S2CSignUpAck;
			newPac->PacketSize = packetSize;
			memcpy(newPac->Body, &ack, sizeof(S2CSignUpAckPacket));

			MainServer::Instance().SendPacketBySessionID(sessionID, newPac);
			});
		});
}