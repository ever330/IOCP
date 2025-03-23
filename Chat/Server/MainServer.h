#pragma once

#include "pch.h"
#include "Define.h"

class IOCP;
class User;
struct PacketBase;

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

private:
	void PacketProcess(unsigned int userID, PacketBase* pac);
	unsigned int GenerateUserID();

private:
	std::shared_ptr<IOCP> m_IOCP;
	std::queue<std::pair<unsigned int, PacketBase*>> m_packets;
	std::mutex m_mutex;
	std::unordered_map<unsigned int, User*> m_users;

	std::unordered_map<unsigned int, unsigned int> m_sessionToUserMap;    // sessionID¿Í userID ¸ÅÄª

	unsigned int m_nextID;
};

