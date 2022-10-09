#include "MyServer.h"

void MyServer::OnConnect(TCPConnection& newConnection)
{
	std::cout << newConnection.ToString() << " - New Connection Accepted." << std::endl;
}

void MyServer::OnDisconnect(TCPConnection& lostConnection, const std::string& reason)
{
	std::cout << "[" << reason << "] Connection lost: " << lostConnection.ToString() << "." << std::endl;
}

bool MyServer::ProcessPacket(std::shared_ptr<Packet> packet, int connectionIndex)
{
	std::string chatmessage;
	uint32_t arraysize = 0;

	switch (packet->GetPacketType())
	{
	default:
		return false;

	}
	return true;
}

