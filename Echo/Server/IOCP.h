#pragma once

#include "pch.h"
#include "Define.h"

struct Session 
{
    SOCKET socket;
    SOCKADDR_IN sockAddr;
    // ���� ���� ���� �߰� ����. ��Ʈ��Ʈ ��.
};

struct IOData
{
    OVERLAPPED overlapped;
    WSABUF wsaBuf;
    char buffer[BUFFER_SIZE];
    int rwMode;
};

class IOCP
{
public:
    void Initialize();

private:
    static DWORD WINAPI WorkerThread(LPVOID lpParam);

private:
    HANDLE m_hIocp;
    std::vector<HANDLE> m_workerThreads;
};