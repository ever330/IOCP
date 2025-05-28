#pragma once

#include "pch.h"
#include "Define.h"
#include "Packet.h"
#include "Vector3.h"
#include "PacketDispatcher.h"

class IOCP;
class User;
class Map;

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

private:
	void PacketWorker();
	void PacketProcess(std::shared_ptr<User> user, PacketBase* pac);
	unsigned int GenerateUserID();

	void RegisterPacketHandlers(); // �ڵ鷯 ��� �Լ�

private:
	std::shared_ptr<IOCP> m_IOCP;
	std::queue<std::pair<unsigned int, PacketBase*>> m_packets;
	mutable std::mutex m_mutex;
	std::unordered_map<unsigned int, std::shared_ptr<User>> m_users;

	std::unordered_map<unsigned int, unsigned int> m_sessionToUserMap;    // sessionID�� userID ��Ī
	std::unordered_map<unsigned int, unsigned int> m_userToSessionMap;    // userID�� sessionID ��Ī

	std::unordered_map<unsigned int, std::shared_ptr<Map>> m_maps;

	unsigned int m_nextID;

	PacketDispatcher m_dispatcher;

	// ��Ŷ ó�� ������ ����.
	std::thread m_packetThread;
	std::atomic<bool> m_isRunning = false;
	std::condition_variable m_condition;
};

