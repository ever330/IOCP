#include "MainServer.h"
#include "IOCP.h"
#include "User.h"
#include "Map.h"

bool MainServer::StartServer()
{
	m_IOCP = std::make_shared<IOCP>();

	m_IOCP->Initialize();

	m_nextID = 1;
	RegisterPacketHandlers();

	m_isRunning = true;
	m_packetThread = std::thread(&MainServer::PacketWorker, this);

	m_maps.emplace(1001, std::make_shared<Map>(1001, m_IOCP));
	m_maps.emplace(1002, std::make_shared<Map>(1002, m_IOCP));
	m_maps.emplace(1003, std::make_shared<Map>(1003, m_IOCP));
	m_maps.emplace(2001, std::make_shared<Map>(2001, m_IOCP));
	m_maps.emplace(2002, std::make_shared<Map>(2002, m_IOCP));
	m_maps.emplace(3001, std::make_shared<Map>(3001, m_IOCP));
	m_maps.emplace(3002, std::make_shared<Map>(3002, m_IOCP));
	m_maps.emplace(4001, std::make_shared<Map>(4001, m_IOCP));
	m_maps.emplace(4002, std::make_shared<Map>(4002, m_IOCP));
	m_maps.emplace(4003, std::make_shared<Map>(4003, m_IOCP));
	m_maps.emplace(5001, std::make_shared<Map>(5001, m_IOCP));

	while (true)
	{
		for (auto& map : m_maps)
		{
			map.second->Update();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	StopServer();

	return false;
}

void MainServer::PushData(unsigned int sessionID, char* data)
{
	// 수신한 데이터에서 패킷 크기 읽어오기
	PacketBase* header = reinterpret_cast<PacketBase*>(data);
	int packetSize = header->PacketSize;

	// 패킷 크기만큼 동적 할당
	PacketBase* pac = reinterpret_cast<PacketBase*>(new char[packetSize]);

	// 전체 데이터 복사 (PacketBase + Body 포함)
	memcpy(pac, data, packetSize);

	// 디버깅 출력
	std::cout << "SessionID : " << sessionID
		<< "\n 받은 패킷 크기: " << pac->PacketSize
		<< ", 타입: " << pac->PacID << std::endl;

	unsigned int curUserID = 0;
	if (m_sessionToUserMap.find(sessionID) != m_sessionToUserMap.end())
	{
		curUserID = m_sessionToUserMap[sessionID];
	}
	else
	{
		curUserID = GenerateUserID();
		m_sessionToUserMap[sessionID] = curUserID;
		m_userToSessionMap[curUserID] = sessionID;
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	m_packets.push(std::make_pair(curUserID, pac));
	m_condition.notify_one(); // 대기 중인 워커 스레드 깨우기
}

void MainServer::StopServer()
{
	m_isRunning = false;
	m_condition.notify_all();  // 모든 스레드 깨우기
	if (m_packetThread.joinable())
		m_packetThread.join();

	m_IOCP->Finalize();
}

void MainServer::DisconnectClient(unsigned int sessionID)
{
	unsigned int curUserID = 0;
	auto it = m_sessionToUserMap.find(sessionID);
	if (it != m_sessionToUserMap.end())
	{
		curUserID = m_sessionToUserMap[sessionID];
		m_users.erase(curUserID);
		m_sessionToUserMap.erase(sessionID);
		m_userToSessionMap.erase(curUserID);
	}
	else
	{
		std::cout << "Disconnect Error!" << std::endl;
	}
}

void MainServer::BroadCast(const std::unordered_set<unsigned int>& userIDs, PacketBase* packet)
{
	for (const auto& userID : userIDs)
	{
		auto it = m_users.find(userID);
		if (it != m_users.end())
		{
			unsigned int sessionID = m_sessionToUserMap[userID];
			m_IOCP->SendPacket(sessionID, reinterpret_cast<char*>(packet), packet->PacketSize);
		}
	}
}

std::shared_ptr<User> MainServer::GetUser(unsigned int userID) const
{
	auto it = m_users.find(userID);
	if (it != m_users.end())
	{
		return it->second;
	}
	return nullptr;
}

void MainServer::PacketWorker()
{
	while (m_isRunning)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		m_condition.wait(lock, [this]() {
			return !m_packets.empty() || !m_isRunning;
			});

		if (!m_isRunning) break;

		auto pair = m_packets.front();
		m_packets.pop();
		lock.unlock();

		PacketProcess(pair.first, pair.second);
	}
}

void MainServer::PacketProcess(unsigned int userID, PacketBase* pac)
{
	auto it = m_packetHandlers.find(pac->PacID);
	if (it != m_packetHandlers.end())
	{
		it->second(userID, pac);
	}
	else
	{
		std::cerr << "알 수 없는 패킷 ID: " << pac->PacID << std::endl;
	}

	delete[] reinterpret_cast<char*>(pac);
}

// IOCP의 SessionID와는 별도의 유저 개별 부여 ID
unsigned int MainServer::GenerateUserID()
{
	// 임시. DB가 있을 경우 유저마다 고유하게 부여된 ID를 사용해야 함
	return m_nextID++;
}

// 현재 사용하는 핸들러가 많지 않기때문에 MainServer에서 처리중 이지만, 추후 더 늘어날 경우 다른 파일로의 분리 필요
void MainServer::RegisterPacketHandlers()
{
	m_packetHandlers[C2SSetName] = [this](unsigned int userID, PacketBase* pac) {
		C2SSetNamePacket setName;
		memcpy(&setName, pac->Body, pac->PacketSize - sizeof(PacketBase));

		std::cout << "이름 설정 : " << setName.Name << std::endl;
		auto newUser = std::make_shared<User>(userID, setName.Name);
		m_users.insert({ newUser->GetUserID(), newUser });

		PacketBase* newPac = reinterpret_cast<PacketBase*>(new char[pac->PacketSize]);
		newPac->PacID = S2CNewUserAlert;
		newPac->PacketSize = pac->PacketSize;
		memcpy(newPac->Body, &setName, newPac->PacketSize - sizeof(PacketBase));

		char buffer[BUFFER_SIZE];
		memcpy(buffer, newPac, newPac->PacketSize);
		m_IOCP->BroadCast(buffer, newPac->PacketSize);

		delete[] reinterpret_cast<char*>(newPac);
		};

	m_packetHandlers[C2SSendMSG] = [this](unsigned int userID, PacketBase* pac) {
		C2SSendMSGPacket msg;
		memcpy(&msg, pac->Body, pac->PacketSize - sizeof(PacketBase));
		std::cout << msg.Name << " : " << msg.MSG << std::endl;

		PacketBase* newPac = reinterpret_cast<PacketBase*>(new char[pac->PacketSize]);
		newPac->PacID = S2CSendMSG;
		newPac->PacketSize = pac->PacketSize;
		memcpy(newPac->Body, &msg, newPac->PacketSize - sizeof(PacketBase));

		char buffer[BUFFER_SIZE];
		memcpy(buffer, newPac, newPac->PacketSize);
		m_IOCP->BroadCast(buffer, newPac->PacketSize);

		delete[] reinterpret_cast<char*>(newPac);
		};

	m_packetHandlers[C2SHeartBeat] = [this](unsigned int userID, PacketBase* pac) {
		unsigned int sessionID = m_sessionToUserMap.find(userID)->first;
		m_IOCP->UpdateHeartBeatTime(sessionID);
		};

	m_packetHandlers[C2SChangeMap] = [this](unsigned int userID, PacketBase* pac) {
		C2SChangeMapPacket changeMap;
		memcpy(&changeMap, pac->Body, pac->PacketSize - sizeof(PacketBase));

		auto user = GetUser(userID);
		if (user == nullptr)
			return;

		// 현재 맵에서 제거
		unsigned int currentMapID = user->GetCurrentMapID();
		auto oldMapIt = m_maps.find(currentMapID);
		if (oldMapIt != m_maps.end())
		{
			oldMapIt->second->RemoveUser(userID);
		}

		S2CChangeMapAckPacket changeMapAck;
		// 새 맵에 추가
		auto newMapIt = m_maps.find(changeMap.MapID);
		if (newMapIt != m_maps.end())
		{
			newMapIt->second->AddUser(userID);

			// 유저의 현재 맵 ID 갱신
			user->SetCurrentMapID(changeMap.MapID);

			changeMapAck.Result = 1; // 성공
			changeMapAck.MapID = changeMap.MapID;
			changeMapAck.SpawnPosX = newMapIt->second->GetUserSpawnPos().x;
			changeMapAck.SpawnPosY = newMapIt->second->GetUserSpawnPos().y;
			changeMapAck.SpawnPosZ = 0.0f;

			user->GetCharacter().Respawn(newMapIt->second->GetUserSpawnPos());
		}
		else
		{
			changeMapAck.Result = 0; // 실패
			changeMapAck.MapID = 0;
			changeMapAck.SpawnPosX = 0.0f;
			changeMapAck.SpawnPosY = 0.0f;
			changeMapAck.SpawnPosZ = 0.0f;
		}

		int packetSize = sizeof(PacketBase) + sizeof(S2CChangeMapAckPacket);
		char* raw = new char[packetSize];


		PacketBase* newPac = reinterpret_cast<PacketBase*>(raw);
		newPac->PacID = S2CChangeMapAck;
		newPac->PacketSize = packetSize;
		memcpy(newPac->Body, &changeMapAck, sizeof(S2CChangeMapAckPacket));

		m_IOCP->SendPacket(m_userToSessionMap[userID], raw, newPac->PacketSize);

		delete[] raw;
		};

	m_packetHandlers[C2SPlayerChat] = [this](unsigned int userID, PacketBase* pac) {
		auto curUser = GetUser(userID);
		auto curMap = m_maps.find(curUser->GetCurrentMapID());

		C2SPlayerChatPacket chatPacket;
		memcpy(&chatPacket, pac->Body, pac->PacketSize - sizeof(PacketBase));

		S2CPlayerChatPacket chatResponse;
		chatResponse.UserID = userID;
		memcpy(chatResponse.Name, curUser->GetUserName().c_str(), sizeof(chatResponse.Name));
		memcpy(chatResponse.ChatMsg, chatPacket.ChatMsg, sizeof(chatResponse.ChatMsg));
		int packetSize = sizeof(PacketBase) + sizeof(S2CPlayerChatPacket);
		char* raw = new char[packetSize];
		PacketBase* newPac = reinterpret_cast<PacketBase*>(raw);
		newPac->PacID = S2CPlayerChat;
		newPac->PacketSize = packetSize;
		memcpy(newPac->Body, &chatResponse, sizeof(S2CPlayerChatPacket));
		
		BroadCast(curMap->second->GetUsers(), newPac);

		delete[] raw;
		};
}