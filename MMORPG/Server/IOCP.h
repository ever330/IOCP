#pragma once

#include "pch.h"
#include "Define.h"
#include "RecvBuffer.h"
#include "BufferPool.h"

struct Session
{
    SOCKET socket = INVALID_SOCKET;
    SOCKADDR_IN sockAddr = {};
    unsigned int sessionID = 0;
    std::chrono::steady_clock::time_point lastHeartbeatTime;
    std::unique_ptr<RecvBuffer> recvBuffer;

    Session() : recvBuffer(std::make_unique<RecvBuffer>()) {}
};

enum IOMode
{
    IO_ACCEPT = 0,
    IO_READ,
    IO_WRITE
};

struct IOData
{
    OVERLAPPED overlapped = {};
    WSABUF wsaBuf = {};
    IOMode mode = IO_READ;

    char* buffer = nullptr;        // 버퍼풀에서 획득한 버퍼
    int bufferSize = 0;            // 버퍼 크기 (반환 시 필요)
    std::shared_ptr<Session> session;
    unsigned int sessionID = 0;

    IOData() = default;

    ~IOData()
    {
        ReleaseBufferToPool();
    }

    // 버퍼풀에서 버퍼 획득
    void AcquireBufferFromPool(int size)
    {
        ReleaseBufferToPool();  // 기존 버퍼 반환
        buffer = AcquireBuffer(size);
        bufferSize = size;
        wsaBuf.buf = buffer;
        wsaBuf.len = size;
    }

    // 버퍼풀에 버퍼 반환
    void ReleaseBufferToPool()
    {
        if (buffer)
        {
            ReleaseBuffer(buffer, bufferSize);
            buffer = nullptr;
            bufferSize = 0;
            wsaBuf.buf = nullptr;
            wsaBuf.len = 0;
        }
    }

    char* GetBuffer() { return buffer; }
};

class IOCP
{
public:
    void Initialize();
    void Finalize();
    void SendPacket(unsigned int sessionID, const char* packet, int byteLength);
    void BroadCast(const char* packet, int byteLength);
    void UpdateHeartBeatTime(unsigned int sessionID);

private:
    void WorkerThread();
    bool PostAccept(std::shared_ptr<IOData> ioData);
    bool PostRecv(std::shared_ptr<Session> session);
    void ProcessReceivedData(std::shared_ptr<Session> session, int bytesTransferred);
    void EraseSession(unsigned int sessionID);
    unsigned int GenerateSessionID();
    void HeartBeatThread();
    void SendHeartBeat(unsigned int sessionID);

    HANDLE m_hIocp = nullptr;
    std::shared_ptr<Session> m_serverSession;
    std::vector<std::thread> m_workerThreads;
    std::unordered_map<unsigned int, std::shared_ptr<Session>> m_sessions;
    std::atomic<unsigned int> m_nextSessionID{ 1 };
    std::atomic<bool> m_isRunning{ true };

    std::thread m_heartBeatThread;

    std::unordered_map<OVERLAPPED*, std::shared_ptr<IOData>> m_ioDataMap;

    std::mutex m_sessionMutex;      // 세션 맵 뮤텍스
    std::mutex m_ioMapMutex;        // IO 데이터 맵 뮤텍스
};
