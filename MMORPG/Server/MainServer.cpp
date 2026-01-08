#include "pch.h"
#include "MainServer.h"
#include "User.h"
#include "Map.h"
#include "ChatPacketHandler.h"
#include "MapPacketHandler.h"
#include "UserPacketHandler.h"
#include "AuthPacketHandler.h"
#include "RedisManager.h"
#include "Config.h"

bool MainServer::StartServer()
{
	// 설정 파일 로드
	if (!ConfigManager::Instance().LoadConfig("server.config"))
	{
		Log("설정 파일 로드 실패 - 기본값 사용");
	}

	const auto& config = ConfigManager::Instance().GetConfig();

	m_IOCP = std::make_unique<IOCP>();
	m_IOCP->Initialize();

	m_DBManager = std::make_unique<DBManager>();
	m_DBManager->Initialize(config.dbHost, config.dbUser, config.dbPassword, config.dbSchema);

	if (RedisManager::Instance().Connect(config.redisHost, config.redisPort, config.redisPassword))
	{
		Log("Redis 초기화 성공");
	}
	else
	{
		Log("Redis 초기화 실패");
	}
	LoadAllCharactersToRedis();

	RegisterPacketHandlers();

	m_isRunning = true;
	m_workerQueues.resize(PACKET_THREAD);

	for (int i = 0; i < PACKET_THREAD; ++i) {
		m_packetWorkers.emplace_back([this, i]() { PacketWorker(i); });
	}

	m_outputThread = std::thread(&MainServer::OutputServerMessages, this);

	m_mapManager.Initialize();

	while (true)
	{
		Update();
	}

	StopServer();

	return false;
}

void MainServer::PushData(unsigned int sessionID, const char* data, int dataLen)
{
	// 패킷 크기 유효성 검사
	if (dataLen < sizeof(PacketBase) || dataLen > RECV_BUFFER_SIZE)
	{
		Log("PushData: 잘못된 패킷 크기: " + std::to_string(dataLen) + " (세션ID: " + std::to_string(sessionID) + ")");
		return;
	}

	const PacketBase* header = reinterpret_cast<const PacketBase*>(data);

	// 패킷 크기 검증 (헤더의 크기와 실제 데이터 크기 비교)
	if (header->PacketSize != dataLen)
	{
		Log("PushData: 패킷 크기 불일치: 헤더=" + std::to_string(header->PacketSize) + ", 실제=" + std::to_string(dataLen));
		return;
	}

	// 하트비트 패킷은 여기서 처리
	if (header->PacID == PacketID::C2SHeartBeat)
	{
		m_IOCP->UpdateHeartBeatTime(sessionID);
		return;
	}

	// 패킷 ID 유효성 검사
	if (header->PacID > S2CExpGain)
	{
		Log("PushData: 처리 불가능한 패킷 ID: " + std::to_string(header->PacID) + " (세션ID: " + std::to_string(sessionID) + ")");
		return;
	}

	// 패킷 복사 (작업 쓰레드에서 처리할 때까지 유지)
	PacketBase* pac = reinterpret_cast<PacketBase*>(new char[dataLen]);
	memcpy(pac, data, dataLen);

	// 디버그용 로그
	Log("PushData: 세션ID " + std::to_string(sessionID) + ", 패킷 크기: " + std::to_string(pac->PacketSize) + ", 타입: " + std::to_string(pac->PacID));

	unsigned int curUserID = RedisManager::Instance().GetUserIDBySessionID(sessionID);

	int index = sessionID % PACKET_THREAD;
	{
		std::lock_guard<std::mutex> lock(m_workerMutexes[index]);
		m_workerQueues[index].push({ curUserID, sessionID, pac });
	}
	m_workerConds[index].notify_one();
}

void MainServer::StopServer()
{
	PeriodSave();

	m_isRunning = false;
	for (int i = 0; i < PACKET_THREAD; ++i)
	{
		m_workerConds[i].notify_all();  // 모든 작업 쓰레드 깨우기
	}

	for (auto& worker : m_packetWorkers)
	{
		if (worker.joinable())
			worker.join();
	}

	m_IOCP->Finalize();
	RedisManager::Instance().Finalize();
}

void MainServer::DisconnectUserBySessionID(unsigned int sessionID)
{
	std::scoped_lock lock(m_mutex);

	unsigned int curUserID = RedisManager::Instance().GetUserIDBySessionID(sessionID);
	if (curUserID != -1)
	{
		auto userIt = m_users.find(curUserID);
		if (userIt != m_users.end())
		{
			auto user = userIt->second;
			if (user && user->GetCurrentMapID())
			{
				m_mapManager.GetMap(user->GetCurrentMapID())->RemoveUser(user);
			}
			UserLevelSave(&userIt->second->GetCharacter());
		}
		m_users.erase(curUserID);
	}
	else
	{
		Log("DisconnectClient: 세션ID " + std::to_string(sessionID) + "에 해당하는 유저 없음");
	}
	RedisManager::Instance().RemoveMapping(sessionID);
}

void MainServer::SendPacket(unsigned int userID, PacketBase* packet)
{
	auto it = m_users.find(userID);
	if (it != m_users.end())
	{
		unsigned int sessionID = RedisManager::Instance().GetSessionIDByUserID(userID);
		m_IOCP->SendPacket(sessionID, reinterpret_cast<const char*>(packet), packet->PacketSize);
	}
	else
	{
		Log("SendPacket: 유저 ID " + std::to_string(userID) + "에 해당하는 유저 없음");
	}
}

void MainServer::SendPacketBySessionID(unsigned int sessionID, PacketBase* packet)
{
	m_IOCP->SendPacket(sessionID, reinterpret_cast<const char*>(packet), packet->PacketSize);
}

void MainServer::BroadCast(const std::unordered_set<unsigned int>& userIDs, PacketBase* packet)
{
	for (const auto& userID : userIDs)
	{
		auto it = m_users.find(userID);
		if (it != m_users.end())
		{
			unsigned int sessionID = RedisManager::Instance().GetSessionIDByUserID(userID);
			m_IOCP->SendPacket(sessionID, reinterpret_cast<const char*>(packet), packet->PacketSize);
		}
	}
}

void MainServer::AddUser(std::shared_ptr<User> user)
{
	std::scoped_lock lock(m_mutex);
	if (m_users.find(user->GetUserID()) == m_users.end())
	{
		m_users[user->GetUserID()] = user;
	}
	else
	{
		Log("AddUser: 유저 ID " + std::to_string(user->GetUserID()) + "가 이미 존재함");
	}
}

void MainServer::BindSession(unsigned int sessionID, std::shared_ptr<User> user)
{
	std::scoped_lock lock(m_mutex);
	unsigned int userID = user->GetUserID();
	RedisManager::Instance().SetSessionMapping(userID, sessionID);
}

std::shared_ptr<User> MainServer::GetUserByID(unsigned int userID) const
{
	std::scoped_lock lock(m_mutex);
	auto it = m_users.find(userID);
	if (it != m_users.end())
		return it->second;
	return nullptr;
}

void MainServer::RequestQuery(const std::string& sql, std::function<void(bool, sql::ResultSet*)> callback)
{
	m_DBManager->RequestQuery(sql, callback);
}

void MainServer::PacketWorker(int index)
{
	while (m_isRunning)
	{
		PacketJob job;

		{
			std::unique_lock<std::mutex> lock(m_workerMutexes[index]);
			m_workerConds[index].wait(lock, [&]() {
				return !m_workerQueues[index].empty() || !m_isRunning;
				});

			if (!m_isRunning && m_workerQueues[index].empty())
				break;

			job = m_workerQueues[index].front();
			m_workerQueues[index].pop();
		}

		auto pacID = job.packet->PacID;

		// 로그인 관련 패킷 별도 처리
		if (IsAuthPacket(pacID))
		{
			PacketProcess(job.sessionID, job.packet);  // 로그인 패킷은 sessionID로 처리
		}
		else
		{
			std::shared_ptr<User> user = nullptr;

			{
				std::lock_guard<std::mutex> lock(m_mutex);
				auto it = m_users.find(job.userID);
				if (it != m_users.end())
					user = it->second;
			}

			if (user)
			{
				PacketProcess(user, job.packet);  // 로그인 이후 User 객체로 처리
			}
			else
			{
				Log("유저 없음: " + std::to_string(job.userID) + " / PacID: " + std::to_string(pacID));
				delete[] reinterpret_cast<char*>(job.packet);
			}
		}
	}
}

void MainServer::PacketProcess(std::shared_ptr<User> user, PacketBase* pac)
{
	if (!m_dispatcher.DispatchPacket(user, pac))
	{
		Log("처리 불가능한 패킷 ID: " + std::to_string(pac->PacID) + " (UserID: " + std::to_string(user->GetUserID()) + ")");
	}

	delete[] reinterpret_cast<char*>(pac);
}

void MainServer::PacketProcess(unsigned int sessionID, PacketBase* pac)
{
	if (!m_dispatcher.DispatchPacket(sessionID, pac))
	{
		Log("처리 불가능한 패킷 ID: " + std::to_string(pac->PacID) + " (SessionID: " + std::to_string(sessionID) + ")");
	}

	delete[] reinterpret_cast<char*>(pac);
}

void MainServer::RegisterPacketHandlers()
{
	m_dispatcher.RegisterUserHandler(std::make_unique<UserPacketHandler>());
	m_dispatcher.RegisterUserHandler(std::make_unique<ChatPacketHandler>(m_mapManager));
	m_dispatcher.RegisterAuthHandler(std::make_unique<AuthPacketHandler>());
	m_dispatcher.RegisterMapHandler(std::make_unique<MapPacketHandler>(m_mapManager));
}

void MainServer::OutputServerMessages()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	while (m_isRunning)
	{
		std::string logMessage;
		if (m_logQueue.try_pop(logMessage))
		{
			// UTF-8 -> Wide 변환 후 출력
			int wideLen = MultiByteToWideChar(CP_UTF8, 0, logMessage.c_str(), -1, nullptr, 0);
			if (wideLen > 0)
			{
				std::wstring wideStr(wideLen, L'\0');
				MultiByteToWideChar(CP_UTF8, 0, logMessage.c_str(), -1, &wideStr[0], wideLen);

				// 콘솔에 출력
				DWORD written;
				WriteConsoleW(hConsole, wideStr.c_str(), (DWORD)(wideStr.length() - 1), &written, nullptr);
				WriteConsoleW(hConsole, L"\n", 1, &written, nullptr);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}

void MainServer::LoadAllCharactersToRedis()
{
	std::vector<CharacterData> allCharacters = m_DBManager->GetAllCharacters();

	for (const auto& ch : allCharacters)
	{
		RedisManager::Instance().UpdateCharacterToRedis(ch.CharacterID, ch.Level, ch.Experience, ch.Name);
	}
	Log("모든 캐릭터 정보를 Redis에 로드했습니다.");
}

bool MainServer::IsAuthPacket(uint16_t packetID) const
{
	return packetID == C2SLogin || packetID == C2SSignUp || packetID == C2SCheckID;
}

void MainServer::PeriodSave()
{
	std::scoped_lock lock(m_mutex);
	for (auto& it : m_users)
	{
		if (!it.second->IsCharacterSet())
		{
			continue;
		}
		if (it.second->GetCharacter().IsDirty())
		{
			m_DBManager->UpdateCharacterLevelAndExpAsync(it.second->GetCharacter().GetID(),
				it.second->GetCharacter().GetLevel(),
				it.second->GetCharacter().GetExp());

			it.second->GetCharacter().ClearDirty();
		}
	}
}

void MainServer::Log(const std::string& message)
{
	m_logQueue.push(message);
}

MapManager& MainServer::GetMapManager()
{
	return m_mapManager;
}

PacketDispatcher& MainServer::GetPacketDispatcher()
{
	return m_dispatcher;
}

void MainServer::Update()
{
	static auto lastTime = std::chrono::steady_clock::now();
	static int tickCount = 0;

	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - lastTime;
	float deltaTime = elapsed.count();

	// 맵 비동기 업데이트
	m_mapManager.Update(deltaTime, tickCount);

	// 100틱 (10초) 마다 DB에 유저 레벨/경험치 저장
	if (tickCount % 100 == 0)
	{
		PeriodSave();
	}

	lastTime = std::chrono::steady_clock::now();

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	tickCount++;
}

void MainServer::SendExpGain(std::shared_ptr<User> user, int expGained)
{
	S2CExpGainPacket expGain{};
	expGain.ExpGained = expGained;
	expGain.TotalExp = user->GetCharacter().GetExp();
	expGain.Level = user->GetCharacter().GetLevel();

	int packetSize = sizeof(PacketBase) + sizeof(S2CExpGainPacket);
	std::shared_ptr<char[]> buffer(new char[packetSize]);

	PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
	newPac->PacID = S2CExpGain;
	newPac->PacketSize = packetSize;

	memcpy(newPac->Body, &expGain, sizeof(S2CExpGainPacket));

	SendPacket(user->GetUserID(), newPac);
}

void MainServer::UserLevelSave(Character* character)
{
	m_DBManager->UpdateCharacterLevelAndExpAsync(character->GetID(), character->GetLevel(), character->GetExp());
	RedisManager::Instance().UpdateCharacterToRedis(character->GetID(), character->GetLevel(), character->GetExp(), character->GetName());
}
