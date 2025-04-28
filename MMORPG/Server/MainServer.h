#pragma once

#include "pch.h"
#include "Define.h"
#include "Packet.h"
#include "Vector3.h"

class IOCP;
class User;
class Map;

class MainServer
{
private:
	MainServer() {}
	MainServer(const MainServer& ref) {}
	MainServer& operator=(const MainServer& ref) {}
	~MainServer() {}

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

	std::shared_ptr<User> GetUser(unsigned int userID) const;

private:
	void PacketWorker();
	void PacketProcess(unsigned int userID, PacketBase* pac);
	unsigned int GenerateUserID();

	void RegisterPacketHandlers(); // 핸들러 등록 함수

private:
	std::shared_ptr<IOCP> m_IOCP;
	std::queue<std::pair<unsigned int, PacketBase*>> m_packets;
	std::mutex m_mutex;
	std::unordered_map<unsigned int, std::shared_ptr<User>> m_users;

	std::unordered_map<unsigned int, unsigned int> m_sessionToUserMap;    // sessionID와 userID 매칭
	std::unordered_map<unsigned int, unsigned int> m_userToSessionMap;    // userID와 sessionID 매칭

	std::unordered_map<unsigned int, std::shared_ptr<Map>> m_maps;

	unsigned int m_nextID;

	std::unordered_map<PacketID, std::function<void(unsigned int, PacketBase*)>> m_packetHandlers;

	// 패킷 처리 스레드 관련.
	std::thread m_packetThread;
	std::atomic<bool> m_isRunning = false;
	std::condition_variable m_condition;
};

