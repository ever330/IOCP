#pragma once

#include "pch.h"
#include "Define.h"

class IOCP;

class MainServer
{
public:
	MainServer();
	~MainServer() {}

public:
	bool StartServer();

public:
	std::shared_ptr<IOCP> m_Iocp;
};

