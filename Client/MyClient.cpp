#include "MyClient.h"

bool MyClient::ProcessPacket(std::shared_ptr<Packet> packet)
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

void MyClient::OnConnect()
{
	std::cout << "Successfully connected to the server!" << std::endl;

	std::shared_ptr<Packet> helloPacket = std::make_shared<Packet>(PacketType::PT_CHATMESSAGE);
	*helloPacket << std::string("Hello from the client!");
	connection.pm_outgoing.Append(helloPacket);
}
