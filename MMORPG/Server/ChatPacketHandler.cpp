#include "ChatPacketHandler.h"
#include "IOCP.h"
#include "MainServer.h" 

ChatPacketHandler::ChatPacketHandler(MapManager& mapManager)
    : m_mapManager(mapManager)
{
}

bool ChatPacketHandler::CanHandle(int packetID) const
{
    return packetID == C2SPlayerChat;
}

void ChatPacketHandler::Handle(std::shared_ptr<User> user, PacketBase* pac)
{
	auto curMap = m_mapManager.GetMap(user->GetCurrentMapID());
    if (curMap == nullptr) return;

    C2SPlayerChatPacket chatPacket;
    memcpy(&chatPacket, pac->Body, pac->PacketSize - sizeof(PacketBase));

    S2CPlayerChatPacket response;
    response.UserID = user->GetUserID();
    memcpy(response.Name, user->GetUserName().c_str(), sizeof(response.Name));
    memcpy(response.ChatMsg, chatPacket.ChatMsg, sizeof(response.ChatMsg));

    int packetSize = sizeof(PacketBase) + sizeof(S2CPlayerChatPacket);
    std::shared_ptr<char[]> buffer(new char[packetSize]);

    PacketBase* newPac = reinterpret_cast<PacketBase*>(buffer.get());
    newPac->PacID = S2CPlayerChat;
    newPac->PacketSize = packetSize;
    memcpy(newPac->Body, &response, sizeof(S2CPlayerChatPacket));

    MainServer::Instance().BroadCast(curMap->GetUsers(), newPac);
}