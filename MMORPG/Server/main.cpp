﻿#include <iostream>
#include "MainServer.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	MainServer::Instance().StartServer();
}