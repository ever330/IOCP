#include "MainServer.h"
#include "User.h"
#include "Map.h"
#include "ChatPacketHandler.h"
#include "MapPacketHandler.h"
#include "UserPacketHandler.h"
#include "AuthPacketHandler.h"

bool MainServer::StartServer()
{
	m_IOCP = std::make_unique<IOCP>();
	m_IOCP->Initialize();

	m_DBManager = std::make_unique<DBManager>();
	m_DBManager->Initialize("tcp://127.0.0.1:3306", "root", "rainbow@@", "rainbow");

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
		m_mapManager.Update();

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

	unsigned int curUserID = 0;
	if (m_sessionToUserMap.find(sessionID) != m_sessionToUserMap.end())
	{
		curUserID = m_sessionToUserMap[sessionID];
	}
	else
	{
		m_sessionToUserMap[sessionID] = curUserID;
		m_userToSessionMap[curUserID] = sessionID;
	}

	int index = sessionID % PACKET_THREAD;
	{
		std::lock_guard<std::mutex> lock(m_workerMutexes[index]);
		m_workerQueues[index].push({ curUserID, sessionID, pac });
	}
	m_workerConds[index].notify_one();
}

void MainServer::StopServer()
{
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
}

void MainServer::DisconnectUserBySessionID(unsigned int sessionID)
{
	std::scoped_lock lock(m_mutex);

	auto sessionIt = m_sessionToUserMap.find(sessionID);
	if (sessionIt != m_sessionToUserMap.end())
	{
		unsigned int curUserID = sessionIt->second;

		auto userIt = m_users.find(curUserID);
		if (userIt != m_users.end())
		{
			auto user = userIt->second;
			if (user && user->GetCurrentMapID())
			{
				m_mapManager.GetMap(user->GetCurrentMapID())->RemoveUser(user);
			}
		}

		m_users.erase(curUserID);
		m_sessionToUserMap.erase(sessionID);
		m_userToSessionMap.erase(curUserID);
	}
	else
	{
		Log("DisconnectClient: ���� ID " + std::to_string(sessionID) + "�� ���� ������ �������� ����");
	}
}

void MainServer::DisconnectUser(unsigned int userID)
{
	std::scoped_lock lock(m_mutex);
	auto it = m_users.find(userID);

	if (it != m_users.end())
	{
		auto user = it->second;
		unsigned int sessionID = m_userToSessionMap.find(userID)->second;

		if (user->GetCurrentMapID())
		{
			m_mapManager.GetMap(user->GetCurrentMapID())->RemoveUser(user);
		}
		m_users.erase(it);
		m_sessionToUserMap.erase(sessionID);
		m_userToSessionMap.erase(userID);
	}
	else
	{
		Log("DisconnectUser: ���� ID " + std::to_string(userID) + "�� ���� ������ �������� ����");
	}
}

void MainServer::SendPacket(unsigned int userID, PacketBase* packet)
{
	std::shared_ptr<char[]> buffer(new char[packet->PacketSize]);
	memcpy(buffer.get(), packet, packet->PacketSize);
	auto it = m_users.find(userID);
	if (it != m_users.end())
	{
		unsigned int sessionID = m_userToSessionMap[userID];
		m_IOCP->SendPacket(sessionID, buffer, packet->PacketSize);
	}
	else
	{
		Log("SendPacket: ���� ID " + std::to_string(userID) + "�� ���� ������ �������� ����");
	}
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
			unsigned int sessionID = m_sessionToUserMap[userID];

			m_IOCP->SendPacket(sessionID, buffer, packet->PacketSize);
		}
	}
}

void MainServer::AddUser(unsigned int sessionID, std::shared_ptr<User> user)
{
	std::scoped_lock lock(m_mutex);
	if (m_users.find(user->GetUserID()) == m_users.end())
	{
		m_users[user->GetUserID()] = user;
		m_sessionToUserMap[sessionID] = user->GetUserID();
		m_userToSessionMap[user->GetUserID()] = sessionID;
	}
	else
	{
		Log("AddUser: ���� ID " + std::to_string(user->GetUserID()) + "�� �̹� �����մϴ�.");
	}
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

		std::shared_ptr<User> user = nullptr;

		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto it = m_users.find(job.userID);
			if (it != m_users.end()) 
			{
				user = it->second;
			}
		}

		if (user) 
		{
			PacketProcess(user, job.packet);
		}
		else if (job.packet->PacID == C2SConnect)
		{
			PacketProcess(job.sessionID, job.packet);
		}
		else 
		{
			Log("������ �������� ����: " + std::to_string(job.userID));
			delete[] reinterpret_cast<char*>(job.packet);
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
		Log("ó���� �� ���� ��Ŷ ID: " + std::to_string(pac->PacID) + " (UserID: " + std::to_string(sessionID) + ")");
	}

	delete[] reinterpret_cast<char*>(pac);
}

void MainServer::RegisterPacketHandlers()
{
	m_dispatcher.RegisterHandler(std::make_unique<UserPacketHandler>());
	m_dispatcher.RegisterHandler(std::make_unique<ChatPacketHandler>(m_mapManager));
	m_dispatcher.RegisterHandler(std::make_unique<MapPacketHandler>(m_mapManager, m_userToSessionMap));
	m_dispatcher.RegisterHandler(std::make_unique<AuthPacketHandler>());
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

void MainServer::Log(const std::string& message)
{
	m_logQueue.push(message);
}