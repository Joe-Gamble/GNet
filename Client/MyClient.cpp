#include "MyClient.h"

bool MyClient::ProcessPacket(std::shared_ptr<Packet> packet)
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

	case PacketType::PT_TRANSFORM:
		std::cout << "Recieved Transform" << std::endl;
		break;

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
