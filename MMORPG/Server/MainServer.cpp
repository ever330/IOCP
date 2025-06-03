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
	m_packetThread = std::thread(&MainServer::PacketWorker, this);

	m_outputThread = std::thread(&MainServer::OutputServerMessages, this);

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

	std::scoped_lock lock(m_mutex);
	m_packets.push(std::make_pair(curUserID, pac));
	m_condition.notify_one(); // ��� ���� ��Ŀ ������ �����
}

void MainServer::StopServer()
{
	m_isRunning = false;
	m_condition.notify_all();  // ��� ������ �����
	if (m_packetThread.joinable())
		m_packetThread.join();

	m_IOCP->Finalize();
}

void MainServer::DisconnectClient(unsigned int sessionID)
{
	std::scoped_lock lock(m_mutex);

	auto it = m_sessionToUserMap.find(sessionID);
	if (it != m_sessionToUserMap.end())
	{
		unsigned int curUserID = it->second;

		// 1. �ʿ��� ����
		auto it = m_users.find(curUserID);
		if (it != m_users.end())
		{
			auto user = it->second;
			if (user && user->GetCurrentMapID())
			{
				m_maps[user->GetCurrentMapID()]->RemoveUser(user);
			}
		}

		// 2. MainServer���� ����
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

		// ���� ���� ����
		std::shared_ptr<User> user = nullptr;
		auto it = m_sessionToUserMap.find(pair.first);
		if (it != m_sessionToUserMap.end()) 
		{
			auto userIt = m_users.find(it->second);
			if (userIt != m_users.end()) 
			{
				user = userIt->second;
			}
			else
			{
				user = std::make_shared<User>(pair.first, "");
				m_users.insert({ pair.first, user });
			}
		}

		lock.unlock();

		if (user)
		{
			PacketProcess(user, pair.second);
		}
		else 
		{
			Log("������ �������� ����: " + std::to_string(pair.first));
			delete[] reinterpret_cast<char*>(pair.second); // ������ ������ ��Ŷ�� ����
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
	m_dispatcher.RegisterHandler(std::make_unique<ChatPacketHandler>(m_IOCP, m_maps));
	m_dispatcher.RegisterHandler(std::make_unique<MapPacketHandler>(m_IOCP, m_maps, m_userToSessionMap));
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
