//Server Code

#include <GNet/IncludeMe.h>
#include <iostream>

using namespace GNet;

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

int main()
{
	if (Network::Initialize())
	{
		std::cout << "Winsock api successfully initialized." << std::endl;

		Socket socket(IPVersion::IPv6);
		if (socket.Create() == GResult::G_SUCCESS)
		{
			std::cout << "Socket successfully created." << std::endl;

			int port = 4790;
			if (socket.Listen(IPEndpoint("::1", port)) == GResult::G_SUCCESS)
			{
				std::cout << "Socket successfully listening on port "<< port << "." << std::endl;
				Socket new_connection;
				if (socket.Accept(new_connection) == GResult::G_SUCCESS)
				{
					std::cout << "New connection accepted." << std::endl;

					Packet packet;

					while (true)
					{
						GResult result = new_connection.Recv(packet);

						if (result != GResult::G_SUCCESS)
						{
							std::cerr << "Could not receive packet" << std::endl;
							break;
						}

						if (!ProcessPacket(packet))
							break;
					}
					new_connection.Close();
				}
				else
				{
					std::cerr << "Failed to accept new connection " << std::endl;
				}
			}
			else
			{
				std::cerr << "Socket could not listen to port " << port << "." << std::endl;
			}

			socket.Close();									
		}
		else
		{
			std::cerr << "Socket failed to create." << std::endl;
		}
	}
	Network::Shutdown();
	system("pause");
	return 0;
}