#include "pch.h"
#include "IOCP.h"
#include "MainServer.h"
#include "Packet.h"

void IOCP::Initialize()
{
    WSADATA wsaData;
    SOCKADDR_IN serverAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        MainServer::Instance().Log("WSAStartup Error");
        return;
    }

    // 버퍼풀 초기화
    InitializeBufferPools();

    SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    if (listenSocket == INVALID_SOCKET)
    {
        MainServer::Instance().Log("서버 소켓 생성 실패: " + std::to_string(WSAGetLastError()));
        WSACleanup();
        return;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        MainServer::Instance().Log("bind 실패: " + std::to_string(WSAGetLastError()));
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        MainServer::Instance().Log("listen 실패: " + std::to_string(WSAGetLastError()));
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    // IOCP 생성
    m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    if (m_hIocp == NULL)
    {
        MainServer::Instance().Log("CreateIoCompletionPort failed with error: " + std::to_string(GetLastError()));
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    MainServer::Instance().Log("서버 시작 (포트: " + std::to_string(PORT) + ")");

    m_serverSession = std::make_shared<Session>();
    m_serverSession->socket = listenSocket;
    m_serverSession->sockAddr = serverAddr;

    // Listen 소켓을 IOCP에 연결
    CreateIoCompletionPort((HANDLE)listenSocket, m_hIocp, 0, 0);

    m_isRunning = true;

    // Worker Thread 생성 (detach 하여 실행)
    for (int i = 0; i < IOCP_THREAD; ++i)
    {
        m_workerThreads.emplace_back(&IOCP::WorkerThread, this);
    }

    // HeartBeat Thread 생성
    m_heartBeatThread = std::thread(&IOCP::HeartBeatThread, this);

    // AcceptEx를 위한 초기 요청 등록
    for (int i = 0; i < 10; ++i)
    {
        auto ioData = std::make_shared<IOData>();
        ioData->AcquireBufferFromPool(LOCAL_ADDR_SIZE + REMOTE_ADDR_SIZE);

        {
            std::lock_guard<std::mutex> lock(m_ioMapMutex);
            m_ioDataMap[&ioData->overlapped] = ioData;
        }

        if (!PostAccept(ioData))
        {
            MainServer::Instance().Log("초기 PostAccept 실패");
        }
    }
}

void IOCP::Finalize()
{
    m_isRunning = false;

    // 모든 Worker Thread에 종료 신호 전송
    for (size_t i = 0; i < m_workerThreads.size(); ++i)
    {
        PostQueuedCompletionStatus(m_hIocp, 0, 0, nullptr);
    }

    // Worker Thread 종료 대기
    for (auto& thread : m_workerThreads)
    {
        if (thread.joinable())
            thread.join();
    }

    // HeartBeat Thread 종료 대기
    if (m_heartBeatThread.joinable())
        m_heartBeatThread.join();

    // 세션 정리
    {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        for (auto& [id, session] : m_sessions)
        {
            if (session->socket != INVALID_SOCKET)
            {
                closesocket(session->socket);
            }
        }
        m_sessions.clear();
    }

    // IO 데이터 맵 정리
    {
        std::lock_guard<std::mutex> lock(m_ioMapMutex);
        m_ioDataMap.clear();
    }

    // 서버 소켓 정리
    if (m_serverSession && m_serverSession->socket != INVALID_SOCKET)
    {
        closesocket(m_serverSession->socket);
    }

    if (m_hIocp)
    {
        CloseHandle(m_hIocp);
        m_hIocp = nullptr;
    }

    // 버퍼풀 정리
    ShutdownBufferPools();

    WSACleanup();

    MainServer::Instance().Log("서버 종료 완료");
}

void IOCP::SendPacket(unsigned int sessionID, const char* packet, int byteLength)
{
    std::shared_ptr<Session> session;

    {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        auto it = m_sessions.find(sessionID);
        if (it == m_sessions.end())
        {
            return;
        }
        session = it->second;
    }

    if (!session || session->socket == INVALID_SOCKET)
    {
            MainServer::Instance().Log("세션 소켓이 유효하지 않습니다: " + std::to_string(sessionID));
        EraseSession(sessionID);
        MainServer::Instance().DisconnectUserBySessionID(sessionID);
        return;
    }

    auto writeIoData = std::make_shared<IOData>();
    writeIoData->AcquireBufferFromPool(byteLength);
    memcpy(writeIoData->GetBuffer(), packet, byteLength);
    writeIoData->wsaBuf.len = byteLength;
    writeIoData->mode = IO_WRITE;
    writeIoData->sessionID = sessionID;

    {
        std::lock_guard<std::mutex> lock(m_ioMapMutex);
        m_ioDataMap[&writeIoData->overlapped] = writeIoData;
    }

    DWORD bytesSent = 0;
    int result = WSASend(session->socket, &writeIoData->wsaBuf, 1, &bytesSent, 0, &writeIoData->overlapped, NULL);

    if (result == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            MainServer::Instance().Log("WSASend 실패: " + std::to_string(error));

            {
                std::lock_guard<std::mutex> lock(m_ioMapMutex);
                m_ioDataMap.erase(&writeIoData->overlapped);
            }

            EraseSession(sessionID);
            MainServer::Instance().DisconnectUserBySessionID(sessionID);
        }
    }
}

void IOCP::BroadCast(const char* packet, int byteLength)
{
    std::vector<std::pair<unsigned int, std::shared_ptr<Session>>> sessionsCopy;

    {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        for (auto& [id, session] : m_sessions)
        {
            sessionsCopy.emplace_back(id, session);
        }
    }

    for (auto& [sessionID, session] : sessionsCopy)
    {
        if (session && session->socket != INVALID_SOCKET)
        {
            SendPacket(sessionID, packet, byteLength);
        }
    }
}

void IOCP::UpdateHeartBeatTime(unsigned int sessionID)
{
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    auto it = m_sessions.find(sessionID);
    if (it != m_sessions.end())
    {
        it->second->lastHeartbeatTime = std::chrono::steady_clock::now();
    }
}

bool IOCP::PostAccept(std::shared_ptr<IOData> ioData)
{
    SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    if (clientSocket == INVALID_SOCKET)
    {
        MainServer::Instance().Log("클라이언트 소켓 생성 실패: " + std::to_string(WSAGetLastError()));
        return false;
    }

    auto client = std::make_shared<Session>();
    client->socket = clientSocket;

    ioData->session = client;
    memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));
    ioData->mode = IO_ACCEPT;

    DWORD dwBytes;
    BOOL result = AcceptEx(m_serverSession->socket, client->socket, ioData->GetBuffer(),
        0, LOCAL_ADDR_SIZE, REMOTE_ADDR_SIZE,
        &dwBytes, &ioData->overlapped);

    if (!result)
    {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            MainServer::Instance().Log("AcceptEx 실패: " + std::to_string(error));
            closesocket(clientSocket);
            return false;
        }
    }

    return true;
}

bool IOCP::PostRecv(std::shared_ptr<Session> session)
{
    if (!session || session->socket == INVALID_SOCKET)
        return false;

    auto ioData = std::make_shared<IOData>();
    ioData->AcquireBufferFromPool(BUFFER_SIZE);
    ioData->mode = IO_READ;
    ioData->session = session;
    ioData->sessionID = session->sessionID;

    {
        std::lock_guard<std::mutex> lock(m_ioMapMutex);
        m_ioDataMap[&ioData->overlapped] = ioData;
    }

    DWORD flags = 0;
    DWORD bytesReceived = 0;
    int result = WSARecv(session->socket, &ioData->wsaBuf, 1, &bytesReceived, &flags, &ioData->overlapped, NULL);

    if (result == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            std::lock_guard<std::mutex> lock(m_ioMapMutex);
            m_ioDataMap.erase(&ioData->overlapped);
            return false;
        }
    }

    return true;
}

void IOCP::ProcessReceivedData(std::shared_ptr<Session> session, int bytesTransferred)
{
    if (!session || !session->recvBuffer)
        return;

    // 수신 버퍼에서 받은 데이터를 링버퍼로 복사
    // (이전 데이터는 IOData의 버퍼에서 받음 - WorkerThread에서 실제 여기로 처리)

    // 완전한 패킷을 받았으면 처리
    char packetBuffer[RECV_BUFFER_SIZE];
    int packetLen = 0;

    while (session->recvBuffer->ReadPacket(packetBuffer, packetLen))
    {
        // 패킷 크기 유효성 검사
        if (packetLen < sizeof(PacketBase) || packetLen > RECV_BUFFER_SIZE)
        {
            MainServer::Instance().Log("잘못된 패킷 크기: " + std::to_string(packetLen));
            continue;
        }

        // MainServer로 패킷 전달
        MainServer::Instance().PushData(session->sessionID, packetBuffer, packetLen);
    }
}

void IOCP::EraseSession(unsigned int sessionID)
{
    // IO 데이터 맵에서 해당 세션 데이터를 삭제
    {
        std::lock_guard<std::mutex> lock(m_ioMapMutex);
        for (auto it = m_ioDataMap.begin(); it != m_ioDataMap.end();)
        {
            if (it->second->sessionID == sessionID)
                it = m_ioDataMap.erase(it);
            else
                ++it;
        }
    }

    // 세션 맵에서 삭제
    {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        auto it = m_sessions.find(sessionID);
        if (it != m_sessions.end())
        {
            if (it->second->socket != INVALID_SOCKET)
            {
                closesocket(it->second->socket);
                it->second->socket = INVALID_SOCKET;
            }
            m_sessions.erase(it);
        }
    }
}

unsigned int IOCP::GenerateSessionID()
{
    return m_nextSessionID.fetch_add(1);
}

void IOCP::HeartBeatThread()
{
    while (m_isRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_INTERVAL_MS));

        if (!m_isRunning)
            break;

        auto now = std::chrono::steady_clock::now();
        std::vector<unsigned int> toErase;

        {
            std::lock_guard<std::mutex> lock(m_sessionMutex);

            for (const auto& [sessionID, session] : m_sessions)
            {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - session->lastHeartbeatTime);
                if (duration.count() > TIMEOUT_MS)
                {
                    toErase.push_back(sessionID);
                }
            }
        }

        // 타임아웃된 세션 정리 및 하트비트 전송
        for (auto sessionID : toErase)
        {
            MainServer::Instance().Log("타임아웃으로 연결 종료: " + std::to_string(sessionID));
            EraseSession(sessionID);
            MainServer::Instance().DisconnectUserBySessionID(sessionID);
        }

        // 하트비트 전송
        std::vector<unsigned int> activeSessionIDs;
        {
            std::lock_guard<std::mutex> lock(m_sessionMutex);
            for (const auto& [sessionID, session] : m_sessions)
            {
                activeSessionIDs.push_back(sessionID);
            }
        }

        for (auto sessionID : activeSessionIDs)
        {
            SendHeartBeat(sessionID);
        }
    }
}

void IOCP::SendHeartBeat(unsigned int sessionID)
{
    int packetSize = sizeof(PacketBase);

    PooledBuffer buffer(packetSize);
    PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.Get());
    packet->PacketSize = packetSize;
    packet->PacID = PacketID::S2CHeartBeat;

    SendPacket(sessionID, buffer.Get(), packetSize);
}

void IOCP::WorkerThread()
{
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    OVERLAPPED* overlapped = nullptr;

    while (m_isRunning)
    {
        BOOL success = GetQueuedCompletionStatus(
            m_hIocp,
            &bytesTransferred,
            &completionKey,
            &overlapped,
            INFINITE
        );

        // 종료 신호 확인
        if (!m_isRunning && overlapped == nullptr)
            break;

        std::shared_ptr<IOData> ioData = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_ioMapMutex);
            auto it = m_ioDataMap.find(overlapped);
            if (it != m_ioDataMap.end())
            {
                ioData = it->second;
                m_ioDataMap.erase(it);
            }
        }

        if (!ioData)
        {
            if (overlapped != nullptr)
            {
                MainServer::Instance().Log("오류: 유효하지 않은 IOData");
            }
            continue;
        }

        // 연결 종료 또는 오류 처리
        if (!success || (bytesTransferred == 0 && ioData->mode != IO_ACCEPT))
        {
            if (ioData->session)
            {
                unsigned int sessionID = ioData->session->sessionID;
                if (sessionID != 0)
                {
                    MainServer::Instance().Log("클라이언트 연결 종료: " + std::to_string(sessionID));
                    EraseSession(sessionID);
                    MainServer::Instance().DisconnectUserBySessionID(sessionID);
                }
            }
            continue;
        }

        switch (ioData->mode)
        {
        case IO_ACCEPT:
        {
            auto& curSession = ioData->session;
            unsigned int sessionID = GenerateSessionID();
            curSession->sessionID = sessionID;
            curSession->lastHeartbeatTime = std::chrono::steady_clock::now();

            setsockopt(curSession->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                (char*)&m_serverSession->socket, sizeof(m_serverSession->socket));

            // 클라이언트 IP 확인
            SOCKADDR_IN clientAddr;
            int addrLen = sizeof(clientAddr);
            getpeername(curSession->socket, (SOCKADDR*)&clientAddr, &addrLen);

            char szIP[64] = { 0 };
            if (InetNtopA(AF_INET, &clientAddr.sin_addr, szIP, sizeof(szIP)))
            {
                MainServer::Instance().Log("클라이언트 접속: " + std::string(szIP) + " (세션 ID: " + std::to_string(sessionID) + ")");
            }

            // IOCP에 소켓 연결
            CreateIoCompletionPort((HANDLE)curSession->socket, m_hIocp, (ULONG_PTR)curSession.get(), 0);

            // 세션 맵에 추가
            {
                std::lock_guard<std::mutex> lock(m_sessionMutex);
                m_sessions[sessionID] = curSession;
            }

            // 수신 대기 시작
            if (!PostRecv(curSession))
            {
                MainServer::Instance().Log("PostRecv 실패: " + std::to_string(WSAGetLastError()));
                EraseSession(sessionID);
                MainServer::Instance().DisconnectUserBySessionID(sessionID);
            }

            // 다음 Accept 대기
            ioData->session = nullptr;
            ioData->AcquireBufferFromPool(LOCAL_ADDR_SIZE + REMOTE_ADDR_SIZE);

            if (PostAccept(ioData))
            {
                std::lock_guard<std::mutex> lock(m_ioMapMutex);
                m_ioDataMap[&ioData->overlapped] = ioData;
            }
            else
            {
                MainServer::Instance().Log("PostAccept 실패: " + std::to_string(WSAGetLastError()));
            }
            break;
        }

        case IO_READ:
        {
            auto session = ioData->session;
            if (session && session->recvBuffer)
            {
                // 받은 데이터를 링버퍼로 복사
                if (!session->recvBuffer->Write(ioData->GetBuffer(), bytesTransferred))
                {
                    MainServer::Instance().Log("링버퍼 오버플로우: " + std::to_string(session->sessionID));
                    EraseSession(session->sessionID);
                    MainServer::Instance().DisconnectUserBySessionID(session->sessionID);
                    break;
                }

                // 완전한 패킷 처리
                ProcessReceivedData(session, bytesTransferred);

                // 다음 수신 대기
                if (!PostRecv(session))
                {
                    MainServer::Instance().Log("PostRecv 실패: " + std::to_string(WSAGetLastError()));
                    EraseSession(session->sessionID);
                    MainServer::Instance().DisconnectUserBySessionID(session->sessionID);
                }
            }
            break;
        }

        case IO_WRITE:
        {
            // 전송 완료 - IOData가 자동으로 정리됨 (shared_ptr)
            break;
        }

        default:
            MainServer::Instance().Log("오류: 알수없는 IOData 모드: " + std::to_string(ioData->mode));
            break;
        }
    }
}
