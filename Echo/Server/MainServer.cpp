#include "MainServer.h"
#include "IOCP.h"

MainServer::MainServer()
{

}

bool MainServer::StartServer()
{
	m_Iocp = std::make_shared<IOCP>();

	m_Iocp->Initialize();

	return false;
}
