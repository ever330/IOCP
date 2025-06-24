#pragma once

#include "pch.h"
#include "Define.h"
#include "Packet.h"
#include "Vector3.h"
#include "PacketDispatcher.h"
#include "MapManager.h"
#include "DBManager.h"
#include "IOCP.h"

class User;
class Map;

struct PacketJob 
{
	unsigned int userID;
	unsigned int sessionID;
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

	bool StartServer();
	void PushData(unsigned int sessionID, char* data);
	void StopServer();
	void DisconnectUserBySessionID(unsigned int sessionID);
	void DisconnectUser(unsigned int userID);
	void SendPacket(unsigned int userID, PacketBase* packet);
	void BroadCast(const std::unordered_set<unsigned int>& userIDs, PacketBase* packet);
	void AddUser(unsigned int sessionID, std::shared_ptr<User> user);

	std::shared_ptr<User> GetUserByID(unsigned int userID) const;

	void RequestQuery(const std::string& sql, std::function<void(bool, sql::ResultSet*)> callback);

	void Log(const std::string& message);

private:
	void PacketWorker(int index);
	void PacketProcess(std::shared_ptr<User> user, PacketBase* pac);
	// 유저 연결, 해제 시 처리
	void PacketProcess(unsigned int sessionID, PacketBase* pac);

	void RegisterPacketHandlers(); // 핸들러 등록 함수

	void OutputServerMessages(); // 서버 메시지 출력 함수

	std::unique_ptr<IOCP> m_IOCP;
	std::unique_ptr<DBManager> m_DBManager;

	mutable std::mutex m_mutex;
	std::unordered_map<unsigned int, std::shared_ptr<User>> m_users;

	std::unordered_map<unsigned int, unsigned int> m_sessionToUserMap;    // sessionID와 userID 매칭
	std::unordered_map<unsigned int, unsigned int> m_userToSessionMap;    // userID와 sessionID 매칭

	concurrency::concurrent_queue<std::string> m_logQueue; // 로그 메시지 큐

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

