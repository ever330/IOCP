#pragma once

#include "pch.h"
#include "Define.h"
#include "Packet.h"
#include "Vector3.h"
#include "PacketDispatcher.h"
#include "MapManager.h"

class IOCP;
class User;
class Map;

struct PacketJob 
{
	unsigned int userID;
	PacketBase* packet;
};

class MainServer
{
private:
	MainServer() {}
	~MainServer() {}

	MainServer(const MainServer&) = delete;
	MainServer& operator=(const MainServer&) = delete;
	MainServer(MainServer&&) = delete;
	MainServer& operator=(MainServer&&) = delete;

public:
	static MainServer& Instance()
	{
		static MainServer s;
		return s;
	}

public:
	bool StartServer();
	void PushData(unsigned int sessionID, char* data);
	void StopServer();
	void DisconnectClient(unsigned int sessionID);
	void BroadCast(const std::unordered_set<unsigned int>& userIDs, PacketBase* packet);

	std::shared_ptr<User> GetUserByID(unsigned int userID) const;

	void Log(const std::string& message); // �α� �޽��� ť�� �߰�

private:
	void PacketWorker(int index);
	void PacketProcess(std::shared_ptr<User> user, PacketBase* pac);
	unsigned int GenerateUserID();

	void RegisterPacketHandlers(); // �ڵ鷯 ��� �Լ�

	void OutputServerMessages(); // ���� �޽��� ��� �Լ�

private:
	std::shared_ptr<IOCP> m_IOCP;
	mutable std::mutex m_mutex;
	std::unordered_map<unsigned int, std::shared_ptr<User>> m_users;

	std::unordered_map<unsigned int, unsigned int> m_sessionToUserMap;    // sessionID�� userID ��Ī
	std::unordered_map<unsigned int, unsigned int> m_userToSessionMap;    // userID�� sessionID ��Ī

	std::mutex m_logMutex; // �α� �޽��� ť ������ ���� ���ؽ�
	concurrency::concurrent_queue<std::string> m_logQueue; // �α� �޽��� ť

	unsigned int m_nextID;

	PacketDispatcher m_dispatcher;
	MapManager m_mapManager;

	// ��Ŷ ó�� ������ ����.
	std::vector<std::thread> m_packetWorkers;
	std::vector<std::queue<PacketJob>> m_workerQueues;
	std::mutex m_workerMutexes[PACKET_THREAD];
	std::condition_variable m_workerConds[PACKET_THREAD];
	std::atomic<bool> m_isRunning = false;

	// ���� �޽��� ��� ����.
	std::thread m_outputThread;
};

