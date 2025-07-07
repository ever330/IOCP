#pragma once

#include "pch.h"
#include "Define.h"

struct Session 
{
    SOCKET socket;
    SOCKADDR_IN sockAddr;
    int sessionID;
    std::chrono::steady_clock::time_point lastHeartbeatTime;
};

struct IOData
{
    OVERLAPPED overlapped;
    WSABUF wsaBuf;
    int mode;

    std::shared_ptr<char[]> packetMemory;
	std::shared_ptr<Session> session;
    int sessionID;

    char* GetBuffer()
    {
        return packetMemory.get();
    }
};

class IOCP
{
public:
    void Initialize();
    void Finalize();
    void SendPacket(unsigned int sessionID, std::shared_ptr<char[]> packet, int byteLength);
    void BroadCast(std::shared_ptr<char[]> packet, int byteLength);
    void UpdateHeartBeatTime(unsigned int sessionID);

private:
    void WorkerThread();
    bool PostAccept(std::shared_ptr<IOData> ioData);
    bool PostRecv(std::shared_ptr<Session> session, std::shared_ptr<IOData> ioData);
    void EraseSession(unsigned int sessionID);
    unsigned int GenerateSessionID();
	void HeartBeatThread();
	void SendHeartBeat(unsigned int sessionID);

    HANDLE m_hIocp;
    std::shared_ptr<Session> m_serverSession;
    std::vector<std::thread> m_workerThreads;
    std::unordered_map<unsigned int, std::shared_ptr<Session>> m_sessions;
    unsigned int m_nextSessionID;
    std::queue<unsigned int> m_availableSessionIDs;

	std::thread m_heartBeatThread;

    std::unordered_map<OVERLAPPED*, std::shared_ptr<IOData>> m_ioDataMap;

    std::mutex m_mutex;
	std::mutex m_ioMapMutex;

    LPFN_ACCEPTEX lpfnAcceptEx;
    GUID GuidAcceptEx;
};