#pragma once

#include "pch.h"
#include "Define.h"

struct Session 
{
    SOCKET socket;
    SOCKADDR_IN sockAddr;
    int sessionIndex;
};

struct IOData
{
    OVERLAPPED overlapped;
    WSABUF wsaBuf;
    char buffer[BUFFER_SIZE];
    int mode;

    Session* session;
};

class IOCP
{
public:
    void Initialize();
    void Finalize();
    void SendPacket(unsigned int sessionID, char* packet, int byteLength);
    void BroadCast(char* packet, int byteLength);

private:
    void WorkerThread();
    bool PostAccept(IOData* ioData);
    bool PostRecv(Session* session, IOData* ioData);
    void EraseSession(unsigned int index);
    unsigned int GenerateSessionID();

private:
    HANDLE m_hIocp;
    Session* m_serverSession;
    std::vector<std::thread> m_workerThreads;
    std::unordered_map<unsigned int, Session*> m_sessions;
    unsigned int m_nextSessionID;
    std::queue<unsigned int> m_availableSessionIDs;

    std::mutex m_mutex;

    LPFN_ACCEPTEX lpfnAcceptEx;
    GUID GuidAcceptEx;
};