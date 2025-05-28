#include "IOCP.h"
#include "MainServer.h"
#include "Packet.h"

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
		std::cerr << "���� ���� ���� ����: " << WSAGetLastError() << std::endl;
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

	// IOCP ����
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (m_hIocp == NULL)
	{
		std::cerr << "CreateIoCompletionPort failed with error: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	std::cout << "���� ����" << std::endl;

	m_serverSession = new Session;
	m_serverSession->socket = listenSocket;
	m_serverSession->sockAddr = serverAddr;

	int nZero = 0;
	int nRet = setsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero));

	if (nRet == SOCKET_ERROR)
	{
		std::cerr << "setsockopt(SNDBUF) ����: " << WSAGetLastError() << std::endl;
		return;
	}

	// Worker Thread ����
	for (int i = 0; i < _Thrd_hardware_concurrency() - CONTENTS_THREAD; ++i)
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
		std::cerr << "WSAIoctl ����: " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	m_nextSessionID = 1;

	std::shared_ptr<IOData> ioData = std::make_shared<IOData>();
	memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));
	ioData->mode = ACCEPT;
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
		delete session.second;
	}

	closesocket(m_serverSession->socket);
	delete m_serverSession;
}

void IOCP::SendPacket(unsigned int sessionID, std::shared_ptr<char[]> packet, int byteLength)
{
	auto it = m_sessions.find(sessionID);
	if (it == m_sessions.end())
	{
		std::cerr << "���� ID�� ã�� �� �����ϴ�: " << sessionID << std::endl;
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
	int result = WSASend(it->second->socket, &(writeIoData->wsaBuf), 1, &bytesSent, 0, &(writeIoData->overlapped), NULL);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			std::cerr << "WSASend ����, error: " << error << std::endl;
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
		writeIoData->packetMemory = std::shared_ptr<char[]>(new char[byteLength]);
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
			std::cerr << "���� ������ ��ȿ���� �ʽ��ϴ�: " << session.first << std::endl;
			EraseSession(session.first);
		}
	}
}

void IOCP::UpdateHeartBeatTime(unsigned int sessionID)
{
	auto it = m_sessions.find(sessionID);
	if (it != m_sessions.end())
	{
		it->second->lastHeartbeatTime = std::chrono::steady_clock::now();
	}
	else
	{
		std::cerr << "���� ID�� ã�� �� �����ϴ�: " << sessionID << std::endl;
	}
}

bool IOCP::PostAccept(std::shared_ptr<IOData> ioData)
{
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (clientSocket == INVALID_SOCKET)
	{
		std::cerr << "Ŭ���̾�Ʈ ���� ���� ����: " << WSAGetLastError() << std::endl;
		return false;
	}

	// Ŭ���̾�Ʈ ���ؽ�Ʈ �Ҵ� �� �ʱ�ȭ
	Session* client = new Session;
	client->socket = clientSocket;

	ioData->session = client;
	memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));
	ioData->mode = ACCEPT;

	DWORD dwBytes;

	bool result = lpfnAcceptEx(m_serverSession->socket, client->socket, ioData->GetBuffer(),
		0,
		LOCAL_ADDR_SIZE, REMOTE_ADDR_SIZE,
		&dwBytes, &(ioData->overlapped));

	if (!result)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING) // WSA_IO_PENDING(997)�� �ƴϸ� ���� ó��
		{
			std::cerr << "AcceptEx ����: " << error << std::endl;
			closesocket(clientSocket);
			delete client;

			return false;
		}
	}

	return true;
}

bool IOCP::PostRecv(Session* session, std::shared_ptr<IOData> ioData)
{
	memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));
	ioData->wsaBuf.buf = ioData->GetBuffer();
	ioData->wsaBuf.len = BUFFER_SIZE;
	ioData->mode = READ;

	DWORD flags = 0;
	DWORD bytesReceived;
	int result = WSARecv(session->socket, &ioData->wsaBuf, 1, &bytesReceived, &flags, &ioData->overlapped, NULL);

	if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		closesocket(session->socket);
		delete session;
		m_ioDataMap.erase(&ioData->overlapped);

		return false;
	}
	return true;
}

void IOCP::EraseSession(unsigned int sessionID)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	for (auto it = m_ioDataMap.begin(); it != m_ioDataMap.end();)
	{
		if (it->second->sessionID == sessionID)
			it = m_ioDataMap.erase(it);
		else
			++it;
	}

	auto it = m_sessions.find(sessionID);
	if (it != m_sessions.end())
	{
		closesocket(it->second->socket);
		delete it->second;
		m_sessions.erase(it);

		MainServer::Instance().DisconnectClient(sessionID);
		// ���� ID ������ ���� ť�� �߰�
		//m_availableSessionIDs.push(sessionID);
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

		std::lock_guard<std::mutex> lock(m_mutex); // ���� ���� ����ȭ  

		for (auto& sessionPair : m_sessions)
		{
			unsigned int sessionID = sessionPair.first;
			Session* session = sessionPair.second;

			// Ÿ�Ӿƿ� üũ  
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - session->lastHeartbeatTime);
			if (duration.count() > TIMEOUT_MS)
			{
				// ���� ����: ���� ���� ó��  
				EraseSession(sessionID);
				continue;
			}

			// ��Ʈ��Ʈ ��Ŷ ����  
			SendHeartBeat(sessionID);
		}
	}
}

void IOCP::SendHeartBeat(unsigned int sessionID)
{
	int packetSize = sizeof(PacketBase);

	// ��Ŷ �޸� �Ҵ�
	std::shared_ptr<char[]> buffer(new char[packetSize]);

	// ��Ŷ ����
	PacketBase* packet = reinterpret_cast<PacketBase*>(buffer.get());
	packet->PacketSize = packetSize;
	packet->PacID = PacketID::S2CHeartBeat;

	// ��Ŷ ����
	SendPacket(sessionID, buffer, packetSize);
}

void IOCP::WorkerThread()
{
	DWORD bytesTransferred;
	Session* session;
	OVERLAPPED* overlapped;
	DWORD flags = 0;

	while (true)
	{
		BOOL success = GetQueuedCompletionStatus(
			m_hIocp,
			&bytesTransferred,
			(PULONG_PTR)&session,
			&overlapped,
			INFINITE
		);

		std::shared_ptr<IOData> ioData;
		{
			std::lock_guard<std::mutex> lock(m_ioMapMutex);
			auto it = m_ioDataMap.find(overlapped);
			if (it != m_ioDataMap.end()) {
				ioData = it->second;
				m_ioDataMap.erase(it);  // ó�� �Ϸ� �� ����
			}
		}

		if (!success || (bytesTransferred == 0 && ioData->mode != ACCEPT))
		{
			std::cout << "Ŭ���̾�Ʈ ���� ����" << std::endl;
			if (session)
			{
				EraseSession(session->sessionID);
			}
			continue;
		}

		if (ioData->mode == ACCEPT)
		{
			Session* curSession = ioData->session;

			unsigned int sessionID = GenerateSessionID();
			curSession->sessionID = sessionID;
			curSession->lastHeartbeatTime = std::chrono::steady_clock::now();

			setsockopt(curSession->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_serverSession->socket, sizeof(m_serverSession->socket));

			int size = sizeof(SOCKADDR_IN);
			SOCKADDR_IN curSockAddrIn;
			memset(&curSockAddrIn, 0x00, sizeof(curSockAddrIn));

			getpeername(curSession->socket, (struct sockaddr*)&curSockAddrIn, &size);

			char szip[64] = { 0 };

			if (InetNtopA(AF_INET, &(&curSockAddrIn)->sin_addr, szip, INET_ADDRSTRLEN) == NULL)
			{
				std::cerr << "IP ��ȯ ����" << std::endl;
				return;
			}

			std::cout << "Ŭ���̾�Ʈ IP: " << szip << " ����" << std::endl;

			CreateIoCompletionPort((HANDLE)curSession->socket, m_hIocp, (ULONG_PTR)curSession, 0);

			m_sessions[sessionID] = curSession;

			std::shared_ptr<IOData> newIoData = std::make_shared<IOData>();
			memset(&newIoData->overlapped, 0, sizeof(OVERLAPPED));
			newIoData->mode = READ;
			newIoData->packetMemory = std::shared_ptr<char[]>(new char[LOCAL_ADDR_SIZE + REMOTE_ADDR_SIZE + 16]);
			newIoData->wsaBuf.buf = ioData->packetMemory.get();
			newIoData->wsaBuf.len = LOCAL_ADDR_SIZE + REMOTE_ADDR_SIZE + 16;

			{
				std::lock_guard<std::mutex> lock(m_ioMapMutex);
				m_ioDataMap[&newIoData->overlapped] = newIoData;
			}

			bool recvResult = PostRecv(curSession, newIoData);
			if (!recvResult)
			{
				std::cerr << "PostRecv() ����!" << std::endl;
				delete curSession;
				m_sessions.erase(sessionID);
				break;
			}

			// ���� Ŭ���̾�Ʈ�� ���� ��û�� �ޱ� ���� PostAccept() ȣ��
			m_ioDataMap[&ioData->overlapped] = ioData;
			bool postResult = PostAccept(ioData);
			if (!postResult)
			{
				std::cerr << "PostAccept() ����!" << std::endl;
				break;
			}
		}
		else if (ioData->mode == READ)
		{
			ioData->wsaBuf.len = bytesTransferred;
			ioData->mode = READ;

			MainServer::Instance().PushData(session->sessionID, ioData->GetBuffer());

			m_ioDataMap[&ioData->overlapped] = ioData;
			bool recvResult = PostRecv(session, ioData);
			if (!recvResult)
			{
				std::cerr << "PostRecv() ����!" << std::endl;
				break;
			}
		}
		else if (ioData->mode == WRITE)
		{

		}
	}
}