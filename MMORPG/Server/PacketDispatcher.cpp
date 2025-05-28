#include "PacketDispatcher.h"
#include "User.h"

void PacketDispatcher::RegisterHandler(std::unique_ptr<IPacketHandler> handler) 
{
    m_handlers.push_back(std::move(handler));
}

bool PacketDispatcher::DispatchPacket(std::shared_ptr<User> user, PacketBase* packet)
{
	for (const auto& handler : m_handlers)
	{
		if (handler->CanHandle(packet->PacID))
		{
			handler->Handle(user, packet);
			return true;
		}
	}
	return false;
}
