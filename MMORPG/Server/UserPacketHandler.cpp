#include "UserPacketHandler.h"
#include "User.h"
#include "MainServer.h"
#include "RedisManager.h"

UserPacketHandler::UserPacketHandler()
{
}

bool UserPacketHandler::CanHandle(uint16_t packetID) const
{
	return packetID == C2SSetName || packetID == C2SPlayerMove || packetID == C2SPlayerStop || packetID == C2SCheckCharacterName || packetID == C2SCreateCharacter || packetID == C2SSelectCharacter || packetID == C2SRanking;
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
	else if (pac->PacID == C2SCheckCharacterName)
	{
		HandleCheckName(user, pac);
	}
	else if (pac->PacID == C2SCreateCharacter)
	{
		HandleCreateCharacter(user, pac);
	}
	else if (pac->PacID == C2SSelectCharacter)
	{
		HandleSelectCharacter(user, pac);
	}
	else if (pac->PacID == C2SRanking)
	{
		HandleRanking(user, pac);
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
		return; // 과거 입력 무시

	user->GetCharacter().SetDirection(move.MoveDirection);
	user->GetCharacter().SetMoving(true);
	user->SetLastInputFrame(move.FrameID); // 클라가 보낸 frameID 저장

	S2CPlayerMovePacket movePacket{};
	movePacket.UserID = user->GetUserID();
	movePacket.MoveDirection = move.MoveDirection;

	int packetSize = sizeof(PacketBase) + sizeof(movePacket);
	std::shared_ptr<char[]> buffer(new char[packetSize]);
	PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
	newPac->PacID = S2CPlayerMove;
	newPac->PacketSize = packetSize;
	memcpy(newPac->Body, &movePacket, sizeof(movePacket));

	// 같은 맵 유저들에게 방향 브로드캐스트
	auto map = MainServer::Instance().GetMapManager().GetMap(user->GetCurrentMapID());

	if (map)
		MainServer::Instance().BroadCast(map->GetUsers(), newPac);


	S2CPlayerPosSyncPacket sync{};
	Vector3 pos = user->GetCharacter().GetPosition();
	sync.PosX = pos.x;
	sync.PosY = pos.y;
	sync.PosZ = pos.z;
	sync.AckFrameID = move.FrameID; // 마지막으로 처리한 클라 입력

	int packetSize2 = sizeof(PacketBase) + sizeof(sync);
	std::shared_ptr<char[]> buffer2(new char[packetSize2]);
	PacketBase* response = reinterpret_cast<PacketBase*>(buffer2.get());
	response->PacID = S2CPlayerPosSync;
	response->PacketSize = packetSize2;
	memcpy(response->Body, &sync, sizeof(sync));

	MainServer::Instance().SendPacket(user->GetUserID(), response); // 유저에게 직접 전송
}

void UserPacketHandler::HandlePlayerStop(std::shared_ptr<User> user, PacketBase* pac)
{
	C2SPlayerStopPacket stop;
	memcpy(&stop, pac->Body, sizeof(stop));

	if (stop.FrameID < user->GetLastInputFrame())
		return; // 과거 입력 무시

	// 플레이어가 멈췄을 때 처리 (예: 이동 중지 플래그 설정)
	user->GetCharacter().SetMoving(false);
	user->SetLastInputFrame(stop.FrameID); // 클라가 보낸 frameID 저장
}

void UserPacketHandler::HandleCheckName(std::shared_ptr<User> user, PacketBase* pac)
{
	C2SCheckCharacterNamePacket checkName;
	memcpy(&checkName, pac->Body, sizeof(checkName));

	std::string characterName = EscapeSQL(std::string(checkName.Name));
	std::string query = "SELECT COUNT(*) AS cnt FROM Characters WHERE Name = '" + characterName + "';";
	unsigned int userID = user->GetUserID();

	MainServer::Instance().RequestQuery(query, [userID](bool success, sql::ResultSet* res) {
		S2CCheckCharacterNameAckPacket ack;
		ack.Result = 1;

		if (success && res && res->next())
		{
			int count = res->getInt("cnt");
			ack.Result = (count == 0) ? 0 : 1;
		}

		delete res;

		int packetSize = sizeof(PacketBase) + sizeof(S2CCheckCharacterNameAckPacket);
		std::shared_ptr<char[]> buffer(new char[packetSize]);

		PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
		newPac->PacID = S2CCheckCharacterNameAck;
		newPac->PacketSize = packetSize;

		memcpy(newPac->Body, &ack, sizeof(S2CCheckCharacterNameAckPacket));
		MainServer::Instance().SendPacket(userID, newPac);
		});
}

void UserPacketHandler::HandleCreateCharacter(std::shared_ptr<User> user, PacketBase* pac)
{
	C2SCreateCharacterPacket createPacket;
	memcpy(&createPacket, pac->Body, sizeof(createPacket));

	std::string name = EscapeSQL(std::string(createPacket.Name));
	uint8_t gender = createPacket.Gender;
	int userID = user->GetUserID();

	std::string insertQuery =
		"INSERT INTO Characters (UserID, Name, Level, Exp, Job, Gender, PosX, PosY, PosZ, MapID) VALUES (" +
		std::to_string(userID) + ", '" + name + "', 1, 0, 0, " + std::to_string(gender) +
		", 100, 100, 0, 1001);";

	MainServer::Instance().RequestQuery(insertQuery, [userID, name](bool insertSuccess, sql::ResultSet* res) {
		delete res;

		S2CCreateCharacterAckPacket ack{};
		ack.Result = 1;
		ack.CharacterID = 0;
		memset(ack.Name, 0, NAME_SIZE);

		if (!insertSuccess)
		{
			// 실패 응답
			int packetSize = sizeof(PacketBase) + sizeof(S2CCreateCharacterAckPacket);
			std::shared_ptr<char[]> buffer(new char[packetSize]);

			PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
			newPac->PacID = S2CCreateCharacterAck;
			newPac->PacketSize = packetSize;
			memcpy(newPac->Body, &ack, sizeof(S2CCreateCharacterAckPacket));

			MainServer::Instance().SendPacket(userID, newPac);
			return;
		}

		// INSERT 성공 → 마지막 삽입된 캐릭터 ID 조회
		std::string lastCharQuery = "SELECT CharID, Gender FROM Characters WHERE UserID = " +
			std::to_string(userID) + " AND Name = '" + name + "' ORDER BY CharID DESC LIMIT 1;";

		MainServer::Instance().RequestQuery(lastCharQuery, [userID, name](bool success2, sql::ResultSet* res2) {
			S2CCreateCharacterAckPacket ack{};
			ack.Result = 1;
			ack.CharacterID = 0;
			ack.Gender = 0;
			memset(ack.Name, 0, NAME_SIZE);

			if (success2 && res2 && res2->next())
			{
				ack.Result = 0;
				ack.CharacterID = static_cast<uint16_t>(res2->getInt("CharID"));
				ack.Gender = static_cast<uint8_t>(res2->getInt("Gender"));
				memcpy(ack.Name, name.c_str(), min(name.size(), static_cast<size_t>(NAME_SIZE - 1)));
			}

			delete res2;

			int packetSize = sizeof(PacketBase) + sizeof(S2CCreateCharacterAckPacket);
			std::shared_ptr<char[]> buffer(new char[packetSize]);

			PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
			newPac->PacID = S2CCreateCharacterAck;
			newPac->PacketSize = packetSize;
			memcpy(newPac->Body, &ack, sizeof(S2CCreateCharacterAckPacket));

			MainServer::Instance().SendPacket(userID, newPac);
			});
		});
}

void UserPacketHandler::HandleSelectCharacter(std::shared_ptr<User> user, PacketBase* pac)
{
	C2SSelectCharacterPacket selectCharacterPacket{};
	memcpy(&selectCharacterPacket, pac->Body, sizeof(selectCharacterPacket));

	uint16_t selectedCharID = selectCharacterPacket.CharacterID;

	// DB에서 선택한 캐릭터 정보 조회
	std::string query = "SELECT * FROM Characters WHERE CharID = " +
		std::to_string(selectedCharID) + " AND UserID = " + std::to_string(user->GetUserID()) + ";";

	MainServer::Instance().RequestQuery(query, [user, selectedCharID](bool success, sql::ResultSet* res) {
		S2CSelectCharacterAckPacket ack{};
		ack.Result = 1; // 실패 기본값
		ack.CharacterID = selectedCharID;
		memset(ack.Name, 0, NAME_SIZE);
		ack.Level = 0;
		ack.Exp = 0;

		if (success && res && res->next())
		{
			// 캐릭터 정보 추출
			ack.Result = 0;
			ack.CharacterID = static_cast<uint16_t>(res->getInt("CharID"));

			std::string name = res->getString("Name");
			memcpy(ack.Name, name.c_str(), min(name.size(), static_cast<size_t>(NAME_SIZE - 1)));

			ack.Level = static_cast<uint16_t>(res->getInt("Level"));
			ack.Exp = static_cast<uint32_t>(res->getInt("Exp"));
			ack.PosX = static_cast<float>(res->getDouble("PosX"));
			ack.PosY = static_cast<float>(res->getDouble("PosY"));
			ack.PosZ = static_cast<float>(res->getDouble("PosZ"));
			ack.MapID = static_cast<uint16_t>(res->getInt("MapID"));

			// 서버 유저 객체에도 선택된 캐릭터를 설정
			user->SetCharacter(ack.CharacterID, name, ack.Level, ack.Exp);
			user->GetCharacter().SetPosition(Vector3(ack.PosX, ack.PosY, ack.PosZ));
			user->SetCurrentMapID(ack.MapID);
		}

		delete res;

		int packetSize = sizeof(PacketBase) + sizeof(S2CSelectCharacterAckPacket);
		std::shared_ptr<char[]> buffer(new char[packetSize]);

		PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
		newPac->PacID = S2CSelectCharacterAck;
		newPac->PacketSize = packetSize;
		memcpy(newPac->Body, &ack, sizeof(S2CSelectCharacterAckPacket));

		MainServer::Instance().SendPacket(user->GetUserID(), newPac);

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		MainServer::Instance().GetPacketDispatcher().GetMapPacketHandler()->ChangeUserMap(user, ack.MapID);
		});

}

void UserPacketHandler::HandleRanking(std::shared_ptr<User> user, PacketBase* pac)
{
	const int maxCount = 10;
	std::vector<S2CRankingInfo> rankers = RedisManager::Instance().GetTopRankingList(maxCount);

	// 내 캐릭터 ID
	uint16_t myCharID = user->GetCharacter().GetID();
	int myRank = RedisManager::Instance().GetCharacterRanking(myCharID); // 0부터 시작
	
	// 검사
	MainServer::Instance().Log("MyRank : " + std::to_string(myRank));
	for (auto& info : rankers)
	{
		MainServer::Instance().Log("Ranker ID : " + std::to_string(info.CharacterID) + ", Name : " + info.Name + ", Level : " + std::to_string(info.Level));
	}

	// 패킷 크기 계산
	const uint8_t count = static_cast<uint8_t>(rankers.size());
	const uint16_t bodySize = sizeof(S2CRankingAckPacket) - sizeof(S2CRankingInfo) * (10 - count);
	const uint16_t packetSize = sizeof(PacketBase) + bodySize;

	std::shared_ptr<char[]> buffer(new char[packetSize]);
	PacketBase* base = reinterpret_cast<PacketBase*>(buffer.get());
	base->PacketSize = packetSize;
	base->PacID = S2CRankingAck;

	S2CRankingAckPacket* res = reinterpret_cast<S2CRankingAckPacket*>(base->Body);
	res->MyRank = (myRank >= 0) ? static_cast<uint16_t>(myRank) : UINT16_MAX;
	res->Count = count;
	memcpy(res->Rankers, rankers.data(), sizeof(S2CRankingInfo) * count);

	MainServer::Instance().SendPacket(user->GetUserID(), base);
}
