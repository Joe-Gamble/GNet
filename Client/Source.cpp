//Client Code

#include <GNet/IncludeMe.h>
#include <iostream>

using namespace GNet;

int main()
{
	if (Network::Initialize())
	{
		std::cout << "Winsock api successfully initialized." << std::endl;

		Socket socket(IPVersion::IPv6);
		//Create the socket
		if (socket.Create() == GResult::G_SUCCESS)
		{
			std::cout << "Socket successfully created." << std::endl;

			//Attempt to connect to a socket listening on Port 4790
			if (socket.Connect(IPEndpoint("::1", 4790)) == GResult::G_SUCCESS)
			{
				std::cout << "Successfully connected to Server!" << std::endl;

				Packet stringPacket(PacketType::PT_CHATMESSAGE);
				stringPacket << std::string("This is a string packet!");

				Packet integersPacket(PacketType::PT_INTEGERARRAY);
				uint32_t arraySize = 6;
				uint32_t integerArray[6] = { 1, 2, 3, 6, 8, 0 };

				integersPacket << arraySize;

				for (auto value : integerArray)
				{
					integersPacket << value;
				}

				while (true)
				{
					GResult result;

					if (rand() % 2 == 0)
					{
						result = socket.Send(stringPacket);
					}
					else
					{
						result = socket.Send(integersPacket);
					}

					if (result != GResult::G_SUCCESS)
					{
						std::cerr << "Could not send packet" << std::endl;
						break;
					}
					
					std::cout << "Attempting to send chunk of data" << std::endl;
					Sleep(500);
				}
			}
			else
			{
				std::cerr << "Failed to connect to the Server" << std::endl;
			}
			socket.Close();
		}
		else
		{
			std::cerr << "Socket failed to create." << std::endl;
		}
	}
	Network::Shutdown();
	system("Pause");
	return 0;
}