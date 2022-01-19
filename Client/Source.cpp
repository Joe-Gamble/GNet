//Client Code

#include <GNet/IncludeMe.h>
#include <iostream>

using namespace GNet;

int main()
{
	if (Network::Initialize())
	{
		std::cout << "Winsock api successfully initialized." << std::endl;

		Socket socket;
		//Create the socket
		if (socket.Create() == GResult::G_SUCCESS)
		{
			std::cout << "Socket successfully created." << std::endl;

			//Attempt to connect to a socket listening on Port 4790
			if (socket.Connect(IPEndpoint("127.0.0.1", 4790)) == GResult::G_SUCCESS)
			{
				std::cout << "Successfully connected to Server!" << std::endl;
				
				std::string buffer = "";
				
				uint32_t a, b, c;
				a = 4;
				b = 6;
				c = 12;

				std::string test = "Hello this is a test string!";

				Packet packet;
				packet << a << b << c;

				while (true)
				{
					GResult result = socket.Send(packet);

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