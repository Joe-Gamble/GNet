//Server Code

#include "Server.h"
#include <iostream>

using namespace GNet;

/*
bool ProcessPacket(Packet& packet)
{
	std::string chatmessage;
	uint32_t arraysize = 0;

	switch (packet.GetPacketType())
	{
	case PacketType::PT_CHATMESSAGE:

		packet >> chatmessage;
		std::cout << "Chat Message: " << chatmessage << std::endl;
		break;

	case PacketType::PT_INTEGERARRAY:

		packet >> arraysize;
		std::cout << "Arraysize: " << arraysize << std::endl;

		for (uint32_t i = 0; i < arraysize; i++)
		{
			uint32_t digit;
			packet >> digit;
			std::cout << "Value [" << i << "] : " << digit << std::endl;
		}
		break;

	default:
		return false;

	}
	return true;
}

*/

int main()
{
	Server server;
	if (server.Initialise(IPEndpoint("::", 6112)))
	{
		while (true)
		{
			server.Frame();
		}
	}

	Network::Shutdown();
	system("pause");
	return 0;
}