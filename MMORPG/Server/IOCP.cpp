#include "IOCP.h"
#include "MainServer.h"
#include "Packet.h"

void IOCP::Initialize()
{
	WSADATA wsaData;

	SOCKADDR_IN serverAddr;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		MainServer::Instance().Log("WSAStartup Error");

	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (listenSocket == INVALID_SOCKET)
	{
		MainServer::Instance().Log("서버 소켓 생성 실패: " + WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);

	bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
	listen(listenSocket, 5);

	// IOCP 생성
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (m_hIocp == NULL)
	{
		MainServer::Instance().Log("CreateIoCompletionPort failed with error: " + WSAGetLastError());
		WSACleanup();
		return;
	}

	MainServer::Instance().Log("서버 시작");

	m_serverSession = std::make_shared<Session>();
	m_serverSession->socket = listenSocket;
	m_serverSession->sockAddr = serverAddr;

	int nZero = 0;
	int nRet = setsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero));

	if (nRet == SOCKET_ERROR)
	{
		MainServer::Instance().Log("setsockopt(SNDBUF) 실패: " + std::to_string(WSAGetLastError()));
		return;
	}

	// Worker Thread 생성
	for (int i = 0; i < IOCP_THREAD; ++i)
	{
		m_workerThreads.emplace_back(std::thread(&IOCP::WorkerThread, this));
	}

	for (auto& worker : m_workerThreads)
	{
		worker.detach();
	}

	CreateIoCompletionPort((HANDLE)listenSocket, m_hIocp, 0, 0);

	m_nextSessionID = 1;

	std::shared_ptr<IOData> ioData = std::make_shared<IOData>();
	ioData->packetMemory = std::shared_ptr<char[]>(new char[LOCAL_ADDR_SIZE + REMOTE_ADDR_SIZE + 16]);
	ioData->wsaBuf.buf = ioData->packetMemory.get();
	ioData->wsaBuf.len = LOCAL_ADDR_SIZE + REMOTE_ADDR_SIZE + 16;

	{
		std::lock_guard<std::mutex> lock(m_ioMapMutex);
		m_ioDataMap[&ioData->overlapped] = ioData;
	}

	bool postResult = PostAccept(ioData);

	if (!postResult)
	{
		return;
	}

	m_heartBeatThread = std::thread(&IOCP::HeartBeatThread, this);
}

void IOCP::Finalize()
{
	for (auto& thread : m_workerThreads)
		PostQueuedCompletionStatus(m_hIocp, 0, 0, nullptr);

	for (auto& thread : m_workerThreads)
		thread.join();

	if (m_hIocp)
		CloseHandle(m_hIocp);

	for (auto& session : m_sessions)
	{
		closesocket(session.second->socket);
	}

	closesocket(m_serverSession->socket);
}

void IOCP::SendPacket(unsigned int sessionID, std::shared_ptr<char[]> packet, int byteLength)
{
	auto it = m_sessions.find(sessionID);
	if (it == m_sessions.end())
	{
		MainServer::Instance().Log("세션 ID를 찾을 수 없습니다: " + std::to_string(sessionID));
		EraseSession(sessionID);
		MainServer::Instance().DisconnectUserBySessionID(sessionID);
		return;
	}

	std::shared_ptr<IOData> writeIoData = std::make_shared<IOData>();
	memset(&writeIoData->overlapped, 0, sizeof(OVERLAPPED));
	std::shared_ptr<char[]> buffer(new char[byteLength]);
	memcpy(buffer.get(), packet.get(), byteLength);
	writeIoData->packetMemory = buffer;
	writeIoData->wsaBuf.buf = writeIoData->GetBuffer();
	writeIoData->wsaBuf.len = byteLength;
	writeIoData->mode = WRITE;

	{
		std::lock_guard<std::mutex> lock(m_ioMapMutex);
		m_ioDataMap[&writeIoData->overlapped] = writeIoData;
	}

	DWORD bytesSent = 0;

	if (it->second->socket == INVALID_SOCKET)
	{
		MainServer::Instance().Log("세션 소켓이 유효하지 않습니다: " + std::to_string(sessionID));
		MainServer::Instance().DisconnectUserBySessionID(sessionID);
		EraseSession(sessionID);
		return;
	}

	int result = WSASend(it->second->socket, &(writeIoData->wsaBuf), 1, &bytesSent, 0, &(writeIoData->overlapped), NULL);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			MainServer::Instance().Log("WSASend 실패: " + std::to_string(error));
			EraseSession(sessionID);
			MainServer::Instance().DisconnectUserBySessionID(sessionID);
			return;
		}
	}
	return;
}

void IOCP::BroadCast(std::shared_ptr<char[]> packet, int byteLength)
{
	for (auto& session : m_sessions)
	{
		std::shared_ptr<IOData> writeIoData = std::make_shared<IOData>();
		memset(&writeIoData->overlapped, 0, sizeof(OVERLAPPED));
		writeIoData->packetMemory = packet;
		writeIoData->wsaBuf.buf = writeIoData->GetBuffer();
		writeIoData->wsaBuf.len = byteLength;
		writeIoData->mode = WRITE;

		{
			std::lock_guard<std::mutex> lock(m_ioMapMutex);
			m_ioDataMap[&writeIoData->overlapped] = writeIoData;
		}

		DWORD bytesSent;
		if (session.second->socket != INVALID_SOCKET)
		{
			WSASend(session.second->socket, &(writeIoData->wsaBuf), 1, &bytesSent, 0, &(writeIoData->overlapped), NULL);
		}
		else
		{
			MainServer::Instance().Log("세션 소켓이 유효하지 않습니다: " + std::to_string(session.first));
			EraseSession(session.first);
			MainServer::Instance().DisconnectUserBySessionID(session.first);
		}
	}
}

void IOCP::UpdateHeartBeatTime(unsigned int sessionID)
{
	std::lock_guard<std::mutex> lock(m_mutex); // 세션 접근 동기화
	auto it = m_sessions.find(sessionID);
	if (it != m_sessions.end())
	{
		it->second->lastHeartbeatTime = std::chrono::steady_clock::now();
	}
	else
	{
		MainServer::Instance().Log("세션 ID를 찾을 수 없습니다: " + std::to_string(sessionID));
		EraseSession(sessionID);
		MainServer::Instance().DisconnectUserBySessionID(sessionID);
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

	// 클라이언트 컨텍스트 할당 및 초기화
	std::shared_ptr<Session> client = std::make_shared<Session>();
	client->socket = clientSocket;

	ioData->session = client;
	memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));
	ioData->mode = ACCEPT;

	DWORD dwBytes;

	bool result = AcceptEx(m_serverSession->socket, client->socket, ioData->GetBuffer(),
		0, LOCAL_ADDR_SIZE, REMOTE_ADDR_SIZE,
		&dwBytes, &(ioData->overlapped));

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

bool IOCP::PostRecv(std::shared_ptr<Session> session, std::shared_ptr<IOData> ioData)
{
	memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));
	ioData->wsaBuf.len = BUFFER_SIZE;
	ioData->wsaBuf.buf = ioData->GetBuffer();
	ioData->mode = READ;

	DWORD flags = 0;
	DWORD bytesReceived;
	int result = WSARecv(session->socket, &ioData->wsaBuf, 1, &bytesReceived, &flags, &ioData->overlapped, NULL);

	if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		return false;
	}
	return true;
}

void IOCP::EraseSession(unsigned int sessionID)
{
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

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_sessions.find(sessionID);
		if (it != m_sessions.end())
		{
			closesocket(it->second->socket);
			m_sessions.erase(it);
		}
	}
}

unsigned int IOCP::GenerateSessionID()
{
	/*std::lock_guard<std::mutex> lock(m_mutex);
	std::cout << "Before check: empty = " << m_availableSessionIDs.empty() << std::endl;
	if (!m_availableSessionIDs.empty())
	{
		unsigned int reusedID = m_availableSessionIDs.front();
		m_availableSessionIDs.pop();
		return reusedID;
	}*/
	return m_nextSessionID++;
}

void IOCP::HeartBeatThread()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_INTERVAL_MS));

		auto now = std::chrono::steady_clock::now();
		std::vector<unsigned int> toErase;

		{
			std::lock_guard<std::mutex> lock(m_mutex);

			for (const auto& [sessionID, session] : m_sessions)
			{
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - session->lastHeartbeatTime);
				if (duration.count() > TIMEOUT_MS)
				{
					toErase.push_back(sessionID); // 삭제 예약
					continue;
				}

				SendHeartBeat(sessionID); // 하트비트 전송
			}
		}

		for (auto sessionID : toErase)
		{
			EraseSession(sessionID);
			MainServer::Instance().Log("세션 타임아웃: " + std::to_string(sessionID));
			MainServer::Instance().DisconnectUserBySessionID(sessionID);
		}
	}
}

void IOCP::SendHeartBeat(unsigned int sessionID)
{
	int packetSize = sizeof(PacketBase);

	// 패킷 메모리 할당
	std::shared_ptr<char[]> buffer(new char[packetSize]);

	// 패킷 생성
	PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.get());
	packet->PacketSize = packetSize;
	packet->PacID = PacketID::S2CHeartBeat;

	// 패킷 전송
	SendPacket(sessionID, buffer, packetSize);
}

void IOCP::WorkerThread()
{
	DWORD bytesTransferred;
	Session* session = nullptr;
	OVERLAPPED* overlapped = nullptr;

	while (true)
	{
		BOOL success = GetQueuedCompletionStatus(
			m_hIocp,
			&bytesTransferred,
			(PULONG_PTR)&session,
			&overlapped,
			INFINITE
		);

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

		if (!success || (bytesTransferred == 0 && (!ioData || ioData->mode != ACCEPT)))
		{
			MainServer::Instance().Log("클라이언트 연결 종료: " + std::to_string(session ? session->sessionID : 0));
			if (session) 
			{
				EraseSession(session->sessionID);
				MainServer::Instance().DisconnectUserBySessionID(session->sessionID);
			}
			continue;
		}

		if (!ioData) 
		{
			MainServer::Instance().Log("오류: 유효하지 않은 IOData (overlapped = nullptr)");
			continue;
		}

		switch (ioData->mode)
		{
		case ACCEPT:
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
			if (!InetNtopA(AF_INET, &clientAddr.sin_addr, szIP, sizeof(szIP))) 
			{
				MainServer::Instance().Log("IP 변환 실패: " + std::to_string(WSAGetLastError()));
				closesocket(curSession->socket);
				break;
			}

			MainServer::Instance().Log("클라이언트 접속: " + std::string(szIP) + " (세션 ID: " + std::to_string(sessionID) + ")");

			CreateIoCompletionPort((HANDLE)curSession->socket, m_hIocp, (ULONG_PTR)curSession.get(), 0);
			m_sessions[sessionID] = curSession;

			auto recvIoData = std::make_shared<IOData>();
			recvIoData->packetMemory = std::shared_ptr<char[]>(new char[BUFFER_SIZE]);
			recvIoData->session = curSession;

			if (PostRecv(curSession, recvIoData)) 
			{
				std::lock_guard<std::mutex> lock(m_ioMapMutex);
				m_ioDataMap[&recvIoData->overlapped] = recvIoData;
			}
			else 
			{
				MainServer::Instance().Log("PostRecv 실패: " + std::to_string(WSAGetLastError()));
				EraseSession(sessionID);
				MainServer::Instance().DisconnectUserBySessionID(sessionID);
				break;
			}

			// 다음 클라이언트 수신 대기
			if (!PostAccept(ioData)) 
			{
				MainServer::Instance().Log("PostAccept 실패: " + std::to_string(WSAGetLastError()));
			}
			else 
			{
				std::lock_guard<std::mutex> lock(m_ioMapMutex);
				m_ioDataMap[&ioData->overlapped] = ioData;
			}
			break;
		}

		case READ:
		{
			MainServer::Instance().PushData(session->sessionID, ioData->GetBuffer());

			if (PostRecv(ioData->session, ioData)) 
			{
				std::lock_guard<std::mutex> lock(m_ioMapMutex);
				m_ioDataMap[&ioData->overlapped] = ioData;
			}
			else 
			{
				MainServer::Instance().Log("PostRecv 실패: " + std::to_string(WSAGetLastError()));
				EraseSession(session->sessionID);
				MainServer::Instance().DisconnectUserBySessionID(session->sessionID);
			}
			break;
		}

		case WRITE:
		{
			// 전송 완료 후 처리
			break;
		}

		default:
			MainServer::Instance().Log("오류: 알 수 없는 IOData 모드");
			break;
		}
	}
}