#include "IOCP.h"

void IOCP::Initialize()
{
	WSADATA wsaData;
	Session client;

	SOCKET serverSock = INVALID_SOCKET;
	SOCKADDR_IN serverAddr;
	int recvBytes, i, flags = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		std::cout << "WSAStartup Error" << std::endl;

	serverSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);

	bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
	listen(serverSock, 5);

	// IOCP 생성
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// Worker Thread 생성
	for (int i = 0; i < _Thrd_hardware_concurrency(); ++i) 
	{
		HANDLE thread = CreateThread(NULL, 0, WorkerThread, m_hIocp, 0, NULL);
		m_workerThreads.push_back(thread);
	}

	std::cout << "서버 시작" << std::endl;

	while (true)
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = accept(serverSock, (SOCKADDR*)&clientAddr, &addrLen);

		// 클라이언트 컨텍스트 할당 및 초기화
		Session* client = new Session;
		IOData* ioData = new IOData;
		client->socket = clientSocket;
		client-> sockAddr = clientAddr;

		// 클라이언트 소켓을 IOCP에 연결
		CreateIoCompletionPort((HANDLE)clientSocket, m_hIocp, (ULONG_PTR)client, 0);

		ioData->wsaBuf.buf = ioData->buffer;
		ioData->wsaBuf.len = BUFFER_SIZE;
		memset(&(ioData->overlapped), 0, sizeof(OVERLAPPED));
		ioData->rwMode = READ;

		WSARecv(client->socket, &(ioData->wsaBuf), 1, (LPDWORD)&recvBytes, (LPDWORD)&flags, &(ioData->overlapped), NULL);
	}
}

DWORD WINAPI IOCP::WorkerThread(LPVOID lpParam) 
{
	HANDLE hComPort = (HANDLE)lpParam;
	DWORD bytesTransferred;
	Session* client;
	IOData* ioData;
	DWORD flags = 0;

	while (true) {
		BOOL success = GetQueuedCompletionStatus(
			hComPort, &bytesTransferred, (PULONG_PTR)&client, (LPOVERLAPPED*)&ioData, INFINITE
		);

		if (!success || bytesTransferred == 0) {
			std::cout << "클라이언트 연결 종료" << std::endl;
			closesocket(client->socket);
			continue;
		}

		if (ioData->rwMode == READ)
		{
			if (bytesTransferred == 0)
			{
				closesocket(client->socket);
				continue;
			}

			memset(&(ioData->overlapped), 0, sizeof(OVERLAPPED));

			std::cout.write(ioData->buffer, bytesTransferred);
			std::cout << std::endl;

			ioData->wsaBuf.len = bytesTransferred;
			ioData->rwMode = WRITE;
			WSASend(client->socket, &(ioData->wsaBuf), 1, NULL, 0, &(ioData->overlapped), NULL);

			ioData = new IOData;
			memset(&(ioData->overlapped), 0, sizeof(OVERLAPPED));
			ioData->wsaBuf.buf = ioData->buffer;
			ioData->wsaBuf.len = sizeof(BUFFER_SIZE);
			ioData->rwMode = READ;

			WSARecv(client->socket, &(ioData->wsaBuf), 1, NULL, &flags, &(ioData->overlapped), NULL);
		}
	}
	return 0;
}