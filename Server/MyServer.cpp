#include "MyServer.h"

void MyServer::OnConnect(TCPConnection& newConnection)
{
	std::cout << newConnection.ToString() << " - New Connection Accepted." << std::endl;

	std::shared_ptr<Packet> welcomeMessagePacket = std::make_shared<Packet>(PacketType::PT_CHATMESSAGE);
	*welcomeMessagePacket << std::string("Welcome!");
	newConnection.pm_outgoing.Append(welcomeMessagePacket);

	std::shared_ptr<Packet> newUserMessagePacket = std::make_unique<Packet>(PacketType::PT_CHATMESSAGE);
	*newUserMessagePacket << std::string("A user has connected!");

	for (auto& connection : connections)
	{
		if (&connection == &newConnection)
			continue;

		connection.pm_outgoing.Append(newUserMessagePacket);
	}
}

void MyServer::OnDisconnect(TCPConnection& lostConnection, std::string reason)
{
	std::cout << "[" << reason << "] Connection lost: " << lostConnection.ToString() << "." << std::endl;

	std::shared_ptr<Packet> connectionLostPacket = std::make_unique<Packet>(PacketType::PT_CHATMESSAGE);
	*connectionLostPacket << std::string("A user has disconnected!");

	for (auto& connection : connections)
	{
		if (&connection == &lostConnection)
			continue;

		connection.pm_outgoing.Append(connectionLostPacket);
	}
}

bool MyServer::ProcessPacket(std::shared_ptr<Packet> packet)
{
	std::string chatmessage;
	uint32_t arraysize = 0;

	switch (packet->GetPacketType())
	{
	case PacketType::PT_CHATMESSAGE:

		*packet >> chatmessage;
		std::cout << "Chat Message: " << chatmessage << std::endl;
		break;

	case PacketType::PT_INTEGERARRAY:

		*packet >> arraysize;
		std::cout << "Arraysize: " << arraysize << std::endl;

		for (uint32_t i = 0; i < arraysize; i++)
		{
			uint32_t digit;
			*packet >> digit;
			std::cout << "Value [" << i << "] : " << digit << std::endl;
		}
		break;

	default:
		return false;

	}
	return true;
}

