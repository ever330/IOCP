#include <iostream>
#include "MainServer.h"

int main()
{
	std::shared_ptr<MainServer> mainServer = std::make_shared<MainServer>();

	mainServer->StartServer();
}