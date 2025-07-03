#include "MainServer.h"
#include "User.h"
#include "Map.h"
#include "ChatPacketHandler.h"
#include "MapPacketHandler.h"
#include "UserPacketHandler.h"
#include "AuthPacketHandler.h"
#include "RedisManager.h"

bool MainServer::StartServer()
{
	m_IOCP = std::make_unique<IOCP>();
	m_IOCP->Initialize();

	m_DBManager = std::make_unique<DBManager>();
	m_DBManager->Initialize("tcp://127.0.0.1:3306", "root", "rainbow@@", "rainbow");

	if (RedisManager::Instance().Connect("127.0.0.1", 6379, "1234"))
	{
		Log("Redis ���� ����");
	}
	else
	{
		Log("Redis ���� ����");
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

void MainServer::PushData(unsigned int sessionID, char* data)
{
	// ������ �����Ϳ��� ��Ŷ ũ�� �о����
	PacketBase* header = reinterpret_cast<PacketBase*>(data);
	int packetSize = header->PacketSize;

	// ��Ŷ ũ�⸸ŭ ���� �Ҵ�
	PacketBase* pac = reinterpret_cast<PacketBase*>(new char[packetSize]);

	// ��ü ������ ���� (PacketBase + Body ����)
	memcpy(pac, data, packetSize);

	// ����� ���
	Log("PushData: ���� ID " + std::to_string(sessionID) + ", ��Ŷ ũ��: " + std::to_string(pac->PacketSize) + ", Ÿ��: " + std::to_string(pac->PacID));

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
		m_workerConds[i].notify_all();  // ��� ������ �����
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
		Log("DisconnectClient: ���� ID " + std::to_string(sessionID) + "�� ���� ������ �������� ����");
	}
	RedisManager::Instance().RemoveMapping(sessionID);
}

void MainServer::SendPacket(unsigned int userID, PacketBase* packet)
{
	std::shared_ptr<char[]> buffer(new char[packet->PacketSize]);
	memcpy(buffer.get(), packet, packet->PacketSize);
	auto it = m_users.find(userID);
	if (it != m_users.end())
	{
		unsigned int sessionID = RedisManager::Instance().GetSessionIDByUserID(userID);
		m_IOCP->SendPacket(sessionID, buffer, packet->PacketSize);
	}
	else
	{
		Log("SendPacket: ���� ID " + std::to_string(userID) + "�� ���� ������ �������� ����");
	}
}

void MainServer::SendPacketBySessionID(unsigned int sessionID, PacketBase* packet)
{
	std::shared_ptr<char[]> buffer(new char[packet->PacketSize]);
	memcpy(buffer.get(), packet, packet->PacketSize);
	m_IOCP->SendPacket(sessionID, buffer, packet->PacketSize);
}

void MainServer::BroadCast(const std::unordered_set<unsigned int>& userIDs, PacketBase* packet)
{
	std::shared_ptr<char[]> buffer(new char[packet->PacketSize]);
	memcpy(buffer.get(), packet, packet->PacketSize);

	for (const auto& userID : userIDs)
	{
		auto it = m_users.find(userID);
		if (it != m_users.end())
		{
			unsigned int sessionID = RedisManager::Instance().GetSessionIDByUserID(userID);
			m_IOCP->SendPacket(sessionID, buffer, packet->PacketSize);
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
		Log("AddUser: ���� ID " + std::to_string(user->GetUserID()) + "�� �̹� �����մϴ�.");
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

		// �α��� �� ��Ŷ ��� ó��
		if (IsAuthPacket(pacID))
		{
			PacketProcess(job.sessionID, job.packet);  // �α��� ��: sessionID�� ó��
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
				PacketProcess(user, job.packet);  // �α��� ��: User ��ü�� ó��
			}
			else
			{
				Log("���� ����: " + std::to_string(job.userID) + " / PacID: " + std::to_string(pacID));
				delete[] reinterpret_cast<char*>(job.packet);
			}
		}
	}
}

void MainServer::PacketProcess(std::shared_ptr<User> user, PacketBase* pac)
{
	if (!m_dispatcher.DispatchPacket(user, pac))
	{
		Log("ó���� �� ���� ��Ŷ ID: " + std::to_string(pac->PacID) + " (UserID: " + std::to_string(user->GetUserID()) + ")");
	}

	delete[] reinterpret_cast<char*>(pac);
}

void MainServer::PacketProcess(unsigned int sessionID, PacketBase* pac)
{
	if (!m_dispatcher.DispatchPacket(sessionID, pac))
	{
		Log("ó���� �� ���� ��Ŷ ID: " + std::to_string(pac->PacID) + " (SessionID: " + std::to_string(sessionID) + ")");
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
	while (m_isRunning)
	{
		std::string logMessage;
		if (m_logQueue.try_pop(logMessage))
		{
			std::cout << logMessage << std::endl; // �α� �޽��� ���
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // ��� �ð� ����
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
	Log("��� ĳ���� ������ Redis�� �ε��߽��ϴ�.");
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
			m_DBManager->UpdateCharacterLevelAndExp(it.second->GetCharacter().GetID(),
				it.second->GetCharacter().GetLevel(),
				it.second->GetCharacter().GetExp());

			it.second->GetCharacter().ClearDirty();
		}
	}
	Log("���� ����, ����ġ. DB����");
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

	// �� ������Ʈ
	m_mapManager.Update(deltaTime, tickCount);

	// 100ƽ (10��)���� DB�� ���� ������ ����ġ ����.
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
	m_DBManager->UpdateCharacterLevelAndExp(character->GetID(), character->GetLevel(), character->GetExp());
	RedisManager::Instance().UpdateCharacterToRedis(character->GetID(), character->GetLevel(), character->GetExp(), character->GetName());
}
