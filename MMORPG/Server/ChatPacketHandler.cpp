#include "ChatPacketHandler.h"
#include "IOCP.h"
#include "MainServer.h" 

ChatPacketHandler::ChatPacketHandler(std::shared_ptr<IOCP> iocp, std::unordered_map<unsigned int, std::shared_ptr<Map>>&maps)
    : m_IOCP(iocp), m_maps(maps) 
{
}

bool ChatPacketHandler::CanHandle(int packetID) const
{
    return packetID == C2SPlayerChat;
}

void ChatPacketHandler::Handle(std::shared_ptr<User> user, PacketBase* packet)
{
    auto mapIt = m_maps.find(user->GetCurrentMapID());
    if (mapIt == m_maps.end()) return;

    C2SPlayerChatPacket chatPacket;
    memcpy(&chatPacket, packet->Body, packet->PacketSize - sizeof(PacketBase));

    S2CPlayerChatPacket response;
    response.UserID = user.get()->GetUserID();
    memcpy(response.Name, user->GetUserName().c_str(), sizeof(response.Name));
    memcpy(response.ChatMsg, chatPacket.ChatMsg, sizeof(response.ChatMsg));

    int packetSize = sizeof(PacketBase) + sizeof(S2CPlayerChatPacket);
    char* raw = new char[packetSize];

    PacketBase* newPac = reinterpret_cast<PacketBase*>(raw);
    newPac->PacID = S2CPlayerChat;
    newPac->PacketSize = packetSize;
    memcpy(newPac->Body, &response, sizeof(S2CPlayerChatPacket));

    MainServer::Instance().BroadCast(mapIt->second->GetUsers(), newPac);

	delete[] raw;
}