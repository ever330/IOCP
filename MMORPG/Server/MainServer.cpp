#include "MainServer.h"
#include "IOCP.h"
#include "User.h"
#include "Map.h"
#include "ChatPacketHandler.h"
#include "MapPacketHandler.h"
#include "UserPacketHandler.h"

bool MainServer::StartServer()
{
	m_IOCP = std::make_shared<IOCP>();

	m_IOCP->Initialize();

	m_nextID = 1;
	RegisterPacketHandlers();

	m_isRunning = true;
	m_workerQueues.resize(PACKET_THREAD);

	for (int i = 0; i < PACKET_THREAD; ++i) {
		m_packetWorkers.emplace_back([this, i]() { PacketWorker(i); });
	}

	m_outputThread = std::thread(&MainServer::OutputServerMessages, this);

	m_mapManager.Initialize(m_IOCP);

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
		curUserID = GenerateUserID();
		m_sessionToUserMap[sessionID] = curUserID;
		m_userToSessionMap[curUserID] = sessionID;
	}

	int index = sessionID % PACKET_THREAD;
	{
		std::lock_guard<std::mutex> lock(m_workerMutexes[index]);
		m_workerQueues[index].push({ curUserID, pac });
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

void MainServer::DisconnectClient(unsigned int sessionID)
{
	std::scoped_lock lock(m_mutex);

	auto it = m_sessionToUserMap.find(sessionID);
	if (it != m_sessionToUserMap.end())
	{
		unsigned int curUserID = it->second;

		auto it = m_users.find(curUserID);
		if (it != m_users.end())
		{
			auto user = it->second;
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

std::shared_ptr<User> MainServer::GetUserByID(unsigned int userID) const
{
	std::scoped_lock lock(m_mutex);
	auto it = m_users.find(userID);
	if (it != m_users.end())
		return it->second;
	return nullptr;
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

		std::shared_ptr<User> user;
		{
			std::lock_guard<std::mutex> lock(m_mutex); // m_users ���� ��ȣ
			auto it = m_users.find(job.userID);
			if (it != m_users.end()) 
			{
				user = it->second;
			}
			else 
			{
				user = std::make_shared<User>(job.userID, "");
				m_users.insert({ job.userID, user });
			}
		}

		if (user) 
		{
			PacketProcess(user, job.packet);
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

// IOCP�� SessionID�ʹ� ������ ���� ���� �ο� ID
unsigned int MainServer::GenerateUserID()
{
	// �ӽ�. DB�� ���� ��� �������� �����ϰ� �ο��� ID�� ����ؾ� ��
	return m_nextID++;
}

void MainServer::RegisterPacketHandlers()
{
	m_dispatcher.RegisterHandler(std::make_unique<UserPacketHandler>(m_IOCP));
	m_dispatcher.RegisterHandler(std::make_unique<ChatPacketHandler>(m_IOCP, m_mapManager));
	m_dispatcher.RegisterHandler(std::make_unique<MapPacketHandler>(m_IOCP, m_mapManager, m_userToSessionMap));
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
	std::scoped_lock lock(m_logMutex);
	std::cout << message << std::endl;
}
