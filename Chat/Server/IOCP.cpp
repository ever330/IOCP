#include "IOCP.h"
#include "MainServer.h"

void IOCP::Initialize()
{
	WSADATA wsaData;

	SOCKADDR_IN serverAddr;
	int recvBytes, i, flags = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		std::cout << "WSAStartup Error" << std::endl;

	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (listenSocket == INVALID_SOCKET) 
	{
		std::cerr << "서버 소켓 생성 실패: " << WSAGetLastError() << std::endl;
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
		std::cerr << "CreateIoCompletionPort failed with error: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	std::cout << "서버 시작" << std::endl;

	m_serverSession = new Session;
	m_serverSession->socket = listenSocket;
	m_serverSession->sockAddr = serverAddr;

	int nZero = 0;
	int nRet = setsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero));

	if (nRet == SOCKET_ERROR)
	{
		std::cerr << "setsockopt(SNDBUF) 실패: " << WSAGetLastError() << std::endl;
		return;
	}

	// Worker Thread 생성
	for (int i = 0; i < _Thrd_hardware_concurrency(); ++i)
	{
		m_workerThreads.emplace_back(std::thread(&IOCP::WorkerThread, this));
	}

	for (auto& worker : m_workerThreads)
	{
		worker.detach();
	}

	CreateIoCompletionPort((HANDLE)listenSocket, m_hIocp, 0, 0);

	lpfnAcceptEx = NULL;
	GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes;

	int result = WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&dwBytes, NULL, NULL);

	if (result == SOCKET_ERROR) 
	{
		std::cerr << "WSAIoctl 실패: " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	m_nextSessionID = 1;

	IOData* ioData = new IOData;
	memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));
	ioData->mode = ACCEPT;
	bool postResult = PostAccept(ioData);

	if (!postResult)
	{
		return;
	}
}

void IOCP::Finalize()
{
	for (auto& session : m_sessions)
	{
		closesocket(session.second->socket);
		delete session.second;
	}

	closesocket(m_serverSession->socket);
	delete m_serverSession;

	CloseHandle(m_hIocp);
}

void IOCP::SendPacket(unsigned int sessionID, char* packet, int byteLength)
{
	
}

void IOCP::BroadCast(char* packet, int byteLength)
{
	for (auto& session : m_sessions)
	{
		IOData* writeIoData = new IOData;
		memset(&writeIoData->overlapped, 0, sizeof(OVERLAPPED));
		writeIoData->wsaBuf.buf = packet;
		writeIoData->wsaBuf.len = byteLength;
		writeIoData->mode = WRITE;
		DWORD bytesSent;
		WSASend(session.second->socket, &(writeIoData->wsaBuf), 1, &bytesSent, 0, &(writeIoData->overlapped), NULL);
	}
}

bool IOCP::PostAccept(IOData* ioData)
{
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (clientSocket == INVALID_SOCKET) 
	{
		std::cerr << "클라이언트 소켓 생성 실패: " << WSAGetLastError() << std::endl;
		return false;
	}

	// 클라이언트 컨텍스트 할당 및 초기화
	Session* client = new Session;
	client->socket = clientSocket;

	ioData->session = client;
	memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));
	ioData->mode = ACCEPT;

	DWORD dwBytes;

	bool result = lpfnAcceptEx(m_serverSession->socket, client->socket, ioData->buffer,
		0,
		LOCAL_ADDR_SIZE, REMOTE_ADDR_SIZE,
		&dwBytes, &(ioData->overlapped));

	if (!result) 
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING) // WSA_IO_PENDING(997)이 아니면 실패 처리
		{  
			std::cerr << "AcceptEx 실패: " << error << std::endl;
			closesocket(clientSocket);
			delete client;

			return false;
		}
	}

	return true;
}

bool IOCP::PostRecv(Session* session, IOData* ioData)
{
	memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));
	ioData->wsaBuf.buf = ioData->buffer;
	ioData->wsaBuf.len = BUFFER_SIZE;
	ioData->mode = READ;

	DWORD flags = 0;
	DWORD bytesReceived;
	int result = WSARecv(session->socket, &ioData->wsaBuf, 1, &bytesReceived, &flags, &ioData->overlapped, NULL);

	if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
	{
		closesocket(session->socket);
		delete session;

		return false;
	}
	return true;
}

void IOCP::EraseSession(unsigned int index)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_sessions.find(index);
	if (it != m_sessions.end())
	{
		closesocket(it->second->socket);
		delete it->second;
		m_sessions.erase(it);

		// 세션 ID 재사용을 위해 큐에 추가
		m_availableSessionIDs.push(index);
	}
}

unsigned int IOCP::GenerateSessionID()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_availableSessionIDs.empty())
	{
		unsigned int reusedID = m_availableSessionIDs.front();
		m_availableSessionIDs.pop();
		return reusedID;
	}
	return m_nextSessionID++;
}

void IOCP::WorkerThread()
{
	DWORD bytesTransferred;
	Session* session;
	IOData* ioData;
	DWORD flags = 0;

	while (true)
	{
		BOOL success = GetQueuedCompletionStatus(
			m_hIocp,
			&bytesTransferred,
			(PULONG_PTR)&session,
			(LPOVERLAPPED*)&ioData,
			INFINITE
		);

		if (!success || (bytesTransferred == 0 && ioData->mode != ACCEPT))
		{
			std::cout << "클라이언트 연결 종료" << std::endl;
			if (session) 
			{
				EraseSession(session->sessionIndex);
			}
			continue;
		}

		if (ioData->mode == ACCEPT)
		{
			Session* curSession = ioData->session;

			unsigned int sessionID = GenerateSessionID();
			curSession->sessionIndex = sessionID;

			setsockopt(curSession->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_serverSession->socket, sizeof(m_serverSession->socket));

			int size = sizeof(SOCKADDR_IN);
			SOCKADDR_IN curSockAddrIn;
			memset(&curSockAddrIn, 0x00, sizeof(curSockAddrIn));

			getpeername(curSession->socket, (struct sockaddr*)&curSockAddrIn, &size);

			char szip[16] = { 0 };

			if (InetNtopA(AF_INET, &(&curSockAddrIn)->sin_addr, szip, INET_ADDRSTRLEN) == NULL) 
			{
				std::cerr << "IP 변환 실패" << std::endl;
				return;
			}

			std::cout << "클라이언트 IP: " << szip << " 접속" << std::endl;

			CreateIoCompletionPort((HANDLE)curSession->socket, m_hIocp, (ULONG_PTR)curSession, 0);

			m_sessions[sessionID] = curSession;

			IOData* newIoData = new IOData;
			memset(&newIoData->overlapped, 0, sizeof(OVERLAPPED));
			newIoData->mode = READ;
			bool recvResult = PostRecv(curSession, newIoData);
			if (!recvResult) 
			{
				std::cerr << "PostRecv() 실패!" << std::endl;
				delete newIoData;
				delete curSession;
				m_sessions.erase(sessionID);
				break;
			}

			// 다음 클라이언트의 연결 요청을 받기 위해 PostAccept() 호출
			bool postResult = PostAccept(ioData);
			if (!postResult) 
			{
				std::cerr << "PostAccept() 실패!" << std::endl;
				delete ioData;
				break;
			}
		}
		else if (ioData->mode == READ)
		{
			ioData->wsaBuf.len = bytesTransferred;
			ioData->mode = READ;

			MainServer::Instance().PushData(session->sessionIndex, ioData->buffer);

			bool recvResult = PostRecv(session, ioData);
			if (!recvResult) 
			{
				std::cerr << "PostRecv() 실패!" << std::endl;
				delete ioData;
				break;
			}
		}
		else if (ioData->mode == WRITE)
		{
			delete ioData;
		}
	}
}