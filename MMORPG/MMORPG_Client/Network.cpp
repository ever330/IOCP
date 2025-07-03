#include "Network.h"

Network::Network()
	: m_socket(INVALID_SOCKET), m_ringBuffer(1024)
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_isRunning = false;
}

Network::~Network()
{
	Disconnect();
	WSACleanup();
}

bool Network::Connect(const std::string& ip, int port)
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET) return false;

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (connect(m_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}

	m_isRunning = true;
	m_recvThread = std::thread(&Network::RecvLoop, this);

	return true;
}

void Network::Disconnect()
{
	if (m_socket != INVALID_SOCKET)
	{
		shutdown(m_socket, SD_BOTH);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	m_isRunning = false;
	if (m_recvThread.joinable())
		m_recvThread.join();
}

void Network::IDCheck(const std::string& name)
{
	C2SCheckIDPacket idCheck;
	strcpy(idCheck.ID, name.c_str());
	PacketBase pac;
	pac.PacID = C2SCheckID;
	pac.PacketSize = sizeof(PacketBase) + sizeof(C2SCheckIDPacket);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &idCheck, sizeof(idCheck));
	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::SignUp(const std::string& name, const std::string& password)
{
	C2SSignUpPacket signUp;
	strcpy(signUp.ID, name.c_str());
	strcpy(signUp.Password, password.c_str());
	PacketBase pac;
	pac.PacID = C2SSignUp;
	pac.PacketSize = sizeof(PacketBase) + sizeof(signUp);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &signUp, sizeof(signUp));
	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::Login(const std::string& name, const std::string& password)
{
	C2SLoginPacket loginPacket;
	strcpy(loginPacket.ID, name.c_str());
	strcpy(loginPacket.Password, password.c_str());
	PacketBase pac;
	pac.PacID = C2SLogin;
	pac.PacketSize = sizeof(PacketBase) + sizeof(loginPacket);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &loginPacket, sizeof(loginPacket));
	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::CharacterNameCheck(const std::string& name)
{
	C2SCheckCharacterNamePacket nameCheckPacket;
	strcpy(nameCheckPacket.Name, name.c_str());
	PacketBase pac;
	pac.PacID = C2SCheckCharacterName;
	pac.PacketSize = sizeof(PacketBase) + sizeof(nameCheckPacket);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &nameCheckPacket, sizeof(nameCheckPacket));
	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::CreateCharacter(const std::string& name, const uint8_t gender)
{
	C2SCreateCharacterPacket createCharacterPacket;
	strcpy(createCharacterPacket.Name, name.c_str());
	createCharacterPacket.Gender = gender;
	PacketBase pac;
	pac.PacID = C2SCreateCharacter;
	pac.PacketSize = sizeof(PacketBase) + sizeof(createCharacterPacket);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &createCharacterPacket, sizeof(createCharacterPacket));
	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::SelectCharacter(const uint16_t characterID)
{
	C2SSelectCharacterPacket selectCharacterPacket;
	selectCharacterPacket.CharacterID = characterID;
	PacketBase pac;
	pac.PacID = C2SSelectCharacter;
	pac.PacketSize = sizeof(PacketBase) + sizeof(selectCharacterPacket);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &selectCharacterPacket, sizeof(selectCharacterPacket));
	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::RequestRanking()
{
	C2SRankingPacket requestRankingPacket;
	PacketBase pac;
	pac.PacID = C2SRanking;
	pac.PacketSize = sizeof(PacketBase) + sizeof(requestRankingPacket);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &requestRankingPacket, sizeof(requestRankingPacket));
	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::SetName(const std::string& name)
{
	C2SSetNamePacket setName;
	strcpy(setName.Name, name.c_str());

	PacketBase pac;
	pac.PacID = C2SSetName;
	pac.PacketSize = sizeof(PacketBase) + sizeof(setName);

	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &setName, sizeof(setName));

	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::SendChat(const std::string& msg)
{
	C2SPlayerChatPacket chat;
	strcpy(chat.ChatMsg, msg.c_str());

	PacketBase pac;
	pac.PacID = C2SPlayerChat;
	pac.PacketSize = sizeof(PacketBase) + sizeof(chat);

	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &chat, sizeof(chat));

	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::ChangeMap(unsigned int mapID)
{
	C2SChangeMapPacket changeMap;
	changeMap.MapID = mapID;

	PacketBase pac;
	pac.PacID = C2SChangeMap;
	pac.PacketSize = sizeof(PacketBase) + sizeof(changeMap);

	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &changeMap, sizeof(changeMap));

	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::ChangeMapByPortal(unsigned int portalID)
{
	C2SChangeMapByPortalPacket changeMapByPortal;
	changeMapByPortal.PortalID = portalID;
	PacketBase pac;
	pac.PacID = C2SChangeMapByPortal;
	pac.PacketSize = sizeof(PacketBase) + sizeof(changeMapByPortal);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &changeMapByPortal, sizeof(changeMapByPortal));
	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::PlayerMove(Direction dir, uint32_t frameID)
{
	C2SPlayerMovePacket playerMove;
	playerMove.MoveDirection = (uint8_t)dir;
	playerMove.FrameID = frameID;

	PacketBase pac;
	pac.PacID = C2SPlayerMove;
	pac.PacketSize = sizeof(PacketBase) + sizeof(playerMove);

	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &playerMove, sizeof(playerMove));

	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::PlayerStop(uint32_t frameID)
{
	C2SPlayerStopPacket playerStop;
	playerStop.FrameID = frameID;
	PacketBase pac;
	pac.PacID = C2SPlayerStop;
	pac.PacketSize = sizeof(PacketBase) + sizeof(playerStop);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &playerStop, sizeof(playerStop));
	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::PlayerAttack(Direction dir)
{
	C2SPlayerAttackPacket playerAttack;
	playerAttack.AttackDirection = (uint8_t)dir;

	PacketBase pac;
	pac.PacID = C2SPlayerAttack;
	pac.PacketSize = sizeof(PacketBase) + sizeof(playerAttack);
	char* buffer = new char[pac.PacketSize];
	memcpy(buffer, &pac, sizeof(pac));
	memcpy(buffer + sizeof(pac), &playerAttack, sizeof(playerAttack));

	send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::RecvLoop()
{
	char tempBuf[1024];
	while (m_isRunning)
	{
		int recvLen = recv(m_socket, tempBuf, sizeof(tempBuf), 0);
		if (recvLen <= 0) break;

		m_ringBuffer.Write(tempBuf, recvLen);

		while (true)
		{
			char header[sizeof(PacketBase)];
			if (!m_ringBuffer.Peek(header, sizeof(header)))
				break;

			PacketBase* pac = reinterpret_cast<PacketBase*>(header);
			if (m_ringBuffer.DataSize() < pac->PacketSize)
				break;

			// 패킷 전체 읽기
			std::vector<char> packetBuf(pac->PacketSize);
			if (!m_ringBuffer.Read(packetBuf.data(), pac->PacketSize))
				break;

			PacketBase* pacRead = reinterpret_cast<PacketBase*>(packetBuf.data());
			char* body = packetBuf.data() + sizeof(PacketBase);

			if (pacRead)
				HandlePacket(pacRead, body);
		}
	}
}

void Network::HandlePacket(PacketBase* pac, const char* body)
{
	if (pac->PacID == S2CCheckIDAck)
	{
		S2CCheckIDAckPacket ack;
		memcpy(&ack, body, sizeof(ack));

		if (m_checkIDResultHandler)
			m_checkIDResultHandler(ack.Result);
	}
	else if (pac->PacID == S2CSignUpAck)
	{
		S2CSignUpAckPacket ack;
		memcpy(&ack, body, sizeof(ack));

		if (m_signUpResultHandler)
			m_signUpResultHandler(ack.Result);
	}
	else if (pac->PacID == S2CLoginAck)
	{
		S2CLoginAckPacket* loginAck = reinterpret_cast<S2CLoginAckPacket*>(pac->Body);
		S2CCharacterInfo* characterList = reinterpret_cast<S2CCharacterInfo*>(pac->Body + sizeof(S2CLoginAckPacket));

		std::vector<S2CCharacterInfo> characters;
		for (int i = 0; i < loginAck->CharacterCount; ++i)
			characters.push_back(characterList[i]);

		if (m_loginResultHandler)
			m_loginResultHandler(loginAck->Result, loginAck->UserID, loginAck->ID, characters);
	}
	else if (pac->PacID == S2CCheckCharacterNameAck)
	{
		S2CCheckCharacterNameAckPacket ack;
		memcpy(&ack, body, sizeof(ack));
		if (m_checkNicknameResultHandler)
			m_checkNicknameResultHandler(ack.Result);
	}
	else if (pac->PacID == S2CCreateCharacterAck)
	{
		S2CCreateCharacterAckPacket ack;
		memcpy(&ack, body, sizeof(ack));
		if (m_createCharacterResultHandler)
			m_createCharacterResultHandler(ack.Result, ack.CharacterID, ack.Gender, ack.Name);
	}
	else if (pac->PacID == S2CSelectCharacterAck)
	{
		S2CSelectCharacterAckPacket ack;
		memcpy(&ack, body, sizeof(ack));
		if (m_selectCharacterResultHandler)
			m_selectCharacterResultHandler(ack.Result, ack.CharacterID, ack.Name, ack.Level, ack.Exp);
	}
	else if (pac->PacID == S2CRankingAck)
	{
		S2CRankingAckPacket ack;
		memcpy(&ack, body, sizeof(ack));
		std::vector<S2CRankingInfo> rankings(ack.Count);
		memcpy(rankings.data(), body + sizeof(uint8_t) + sizeof(uint16_t), sizeof(S2CRankingInfo) * ack.Count);
		if (m_rankingResultHandler)
			m_rankingResultHandler(ack.MyRank, rankings);
	}
	else if (pac->PacID == S2CPlayerChat)
	{
		S2CPlayerChatPacket data;
		memcpy(&data, body, sizeof(data));
		std::string name(data.Name, strnlen(data.Name, NAME_SIZE));
		std::string msg(data.ChatMsg, strnlen(data.ChatMsg, MSG_SIZE));
		if (m_messageHandler)
			m_messageHandler(name + ": " + msg);
	}
	else if (pac->PacID == S2CChangeMapAck)
	{
		if (m_mapChangeHandler)
			m_mapChangeHandler(pac);
	}
	else if (pac->PacID == S2CMonsterState)
	{
		const S2CMonsterStatePacket* state = reinterpret_cast<const S2CMonsterStatePacket*>(body);
		const size_t expectedSize = sizeof(PacketBase) + sizeof(S2CMonsterStatePacket) + sizeof(S2CMonsterStateInfo) * state->MonsterCount;

		if (pac->PacketSize != expectedSize)
		{
			// 유효하지 않은 패킷, 로그 출력하거나 무시
			std::cout << "패킷 에러" << std::endl;
			return;
		}

		std::vector<S2CMonsterStateInfo> monsters(state->MonsterCount);
		memcpy(monsters.data(), body + sizeof(S2CMonsterStatePacket), sizeof(S2CMonsterStateInfo) * state->MonsterCount);

		if (m_monsterInfoHandler)
			m_monsterInfoHandler(monsters);
	}
	else if (pac->PacID == S2CPlayerEnter)
	{
		S2CPlayerEnterPacket playerEnter;
		memcpy(&playerEnter, body, sizeof(playerEnter));
		if (m_playerEnterHandler)
			m_playerEnterHandler(playerEnter.UserID, playerEnter.Name, playerEnter.SpawnPosX, playerEnter.SpawnPosY);
	}
	else if (pac->PacID == S2CPlayerLeave)
	{
		S2CPlayerLeavePacket playerLeave;
		memcpy(&playerLeave, body, sizeof(playerLeave));
		if (m_playerLeaveHandler)
			m_playerLeaveHandler(playerLeave.UserID);
	}
	else if (pac->PacID == S2CPlayerState)
	{
		const S2CPlayerStatePacket* state = reinterpret_cast<const S2CPlayerStatePacket*>(body);
		const size_t expectedSize = sizeof(PacketBase) + sizeof(S2CPlayerStatePacket) + sizeof(S2CPlayerStateInfo) * state->PlayerCount;

		if (pac->PacketSize != expectedSize)
		{
			// 유효하지 않은 패킷, 로그 출력하거나 무시
			std::cout << "패킷 에러" << std::endl;
			return;
		}

		std::vector<S2CPlayerStateInfo> users(state->PlayerCount);
		memcpy(users.data(), body + sizeof(S2CPlayerStatePacket), sizeof(S2CPlayerStateInfo) * state->PlayerCount);

		if (m_playerInfoHandler)
			m_playerInfoHandler(users);
	}
	else if (pac->PacID == S2CPlayerMove)
	{
		const S2CPlayerMovePacket* moveInfo = reinterpret_cast<const S2CPlayerMovePacket*>(body);
		if (m_playerMoveHandler)
			m_playerMoveHandler(moveInfo->UserID, (Direction)moveInfo->MoveDirection);
	}
	else if (pac->PacID == S2CPlayerPosSync)
	{
		const S2CPlayerPosSyncPacket* posSyncInfo = reinterpret_cast<const S2CPlayerPosSyncPacket*>(body);
		if (m_playerPosSyncHandler)
			m_playerPosSyncHandler(posSyncInfo->PosX, posSyncInfo->PosY, posSyncInfo->PosZ, posSyncInfo->AckFrameID);
	}
	else if (pac->PacID == S2CMonsterHit)
	{
		const S2CMonsterHitPacket* hitInfo = reinterpret_cast<const S2CMonsterHitPacket*>(body);
		const size_t expectedSize = sizeof(PacketBase) + sizeof(S2CMonsterHitPacket) + sizeof(S2CMonsterHitInfo) * hitInfo->MonsterCount;

		if (hitInfo->MonsterCount == 0)
		{
			// 몬스터가 없을 경우, 아무것도 하지 않음
			return;
		}

		if (pac->PacketSize != expectedSize)
		{
			// 유효하지 않은 패킷, 로그 출력하거나 무시
			std::cout << "패킷 에러" << std::endl;
			return;
		}

		std::vector<S2CMonsterHitInfo> hits(hitInfo->MonsterCount);
		memcpy(hits.data(), body + sizeof(S2CMonsterHitPacket), sizeof(S2CMonsterHitInfo) * hitInfo->MonsterCount);

		if (m_monsterHitInfoHandler)
			m_monsterHitInfoHandler(hits);
	}
	else if (pac->PacID == S2CMonsterRespawn)
	{
		const S2CMonsterRespawnPacket* respawnInfo = reinterpret_cast<const S2CMonsterRespawnPacket*>(body);
		const size_t expectedSize = sizeof(PacketBase) + sizeof(S2CMonsterRespawnPacket) + sizeof(S2CMonsterRespawnInfo) * respawnInfo->MonsterCount;

		if (respawnInfo->MonsterCount == 0)
		{
			// 몬스터가 없을 경우, 아무것도 하지 않음
			return;
		}
		if (pac->PacketSize != expectedSize)
		{
			// 유효하지 않은 패킷, 로그 출력하거나 무시
			std::cout << "패킷 에러" << std::endl;
			return;
		}

		std::vector<S2CMonsterRespawnInfo> respawns(respawnInfo->MonsterCount);
		memcpy(respawns.data(), body + sizeof(S2CMonsterRespawnPacket), sizeof(S2CMonsterRespawnInfo) * respawnInfo->MonsterCount);

		if (m_monsterRespawnHandler)
			m_monsterRespawnHandler(respawns);
	}
	else if (pac->PacID == S2CPlayerAttack)
	{
		const S2CPlayerAttackPacket* attackInfo = reinterpret_cast<const S2CPlayerAttackPacket*>(body);
		if (m_playerAttackHandler)
			m_playerAttackHandler(attackInfo->UserID, (Direction)attackInfo->AttackDirection);
	}
	else if (pac->PacID == S2CExpGain)
	{
		const S2CExpGainPacket* expInfo = reinterpret_cast<const S2CExpGainPacket*>(body);
		if (m_expGainHandler)
			m_expGainHandler(expInfo->ExpGained, expInfo->TotalExp, expInfo->Level);
	}
	else
	{
		// 알 수 없는 패킷, 로그 출력하거나 무시
		std::cout << "알 수 없는 패킷 ID: " << pac->PacID << std::endl;
	}
}

void Network::SetCheckIDResultCallback(CheckIDResultHandler handler)
{
	m_checkIDResultHandler = handler;
}

void Network::SetSignUpResultCallback(SignUpResultHandler handler)
{
	m_signUpResultHandler = handler;
}

void Network::SetLoginResultCallback(LoginResultHandler handler)
{
	m_loginResultHandler = handler;
}

void Network::SetCheckNicknameResultCallback(CheckNicknameResultHandler handler)
{
	m_checkNicknameResultHandler = handler;
}

void Network::SetCreateCharacterResultCallback(CreateCharacterResultHandler handler)
{
	m_createCharacterResultHandler = handler;
}

void Network::SetSelectCharacterResultCallback(SelecteCharacterResultHandler handler)
{
	m_selectCharacterResultHandler = handler;
}

void Network::SetRankingResultCallback(RankingResultHandler handler)
{
	m_rankingResultHandler = handler;
}

void Network::SetMessageCallback(MessageHandler handler)
{
	m_messageHandler = handler;
}

void Network::SetMapChangeCallback(MapChangeHandler handler)
{
	m_mapChangeHandler = handler;
}

void Network::SetMonsterInfoCallback(MonsterStateInfoHandler handler)
{
	m_monsterInfoHandler = handler;
}

void Network::SetPlayerEnterCallback(PlayerEnterHandler handler)
{
	m_playerEnterHandler = handler;
}

void Network::SetPlayerLeaveCallback(PlayerLeaveHandler handler)
{
	m_playerLeaveHandler = handler;
}

void Network::SetPlayerInfoCallback(PlayerInfoHandler handler)
{
	m_playerInfoHandler = handler;
}

void Network::SetPlayerMoveCallback(PlayerMoveHandler handler)
{
	m_playerMoveHandler = handler;
}

void Network::SetPlayerPosSyncCallback(PlayerPosSyncHandler handler)
{
	m_playerPosSyncHandler = handler;
}

void Network::SetMonsterHitInfoCallback(MonsterHitInfoHandler handler)
{
	m_monsterHitInfoHandler = handler;
}

void Network::SetMonsterRespawnCallback(MonsterRespawnHandler handler)
{
	m_monsterRespawnHandler = handler;
}

void Network::SetPlayerAttackCallback(PlayerAttackHandler handler)
{
	m_playerAttackHandler = handler;
}

void Network::SetExpGainCallback(ExpGainHandler handler)
{
	m_expGainHandler = handler;
}
