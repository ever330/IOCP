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

	void Log(const std::string& message); // 로그 메시지 큐에 추가

private:
	void PacketWorker(int index);
	void PacketProcess(std::shared_ptr<User> user, PacketBase* pac);
	unsigned int GenerateUserID();

	void RegisterPacketHandlers(); // 핸들러 등록 함수

	void OutputServerMessages(); // 서버 메시지 출력 함수

private:
	std::shared_ptr<IOCP> m_IOCP;
	mutable std::mutex m_mutex;
	std::unordered_map<unsigned int, std::shared_ptr<User>> m_users;

	std::unordered_map<unsigned int, unsigned int> m_sessionToUserMap;    // sessionID와 userID 매칭
	std::unordered_map<unsigned int, unsigned int> m_userToSessionMap;    // userID와 sessionID 매칭

	std::mutex m_logMutex; // 로그 메시지 큐 접근을 위한 뮤텍스
	concurrency::concurrent_queue<std::string> m_logQueue; // 로그 메시지 큐

	unsigned int m_nextID;

	PacketDispatcher m_dispatcher;
	MapManager m_mapManager;

	// 패킷 처리 스레드 관련.
	std::vector<std::thread> m_packetWorkers;
	std::vector<std::queue<PacketJob>> m_workerQueues;
	std::mutex m_workerMutexes[PACKET_THREAD];
	std::condition_variable m_workerConds[PACKET_THREAD];
	std::atomic<bool> m_isRunning = false;

	// 서버 메시지 출력 관련.
	std::thread m_outputThread;
};

