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
				char buffer[256];
				strcpy_s(buffer, "Hello world from client! \0");

				int bytesSent = 0;

				GResult result = GResult::G_SUCCESS;
				while (result == GResult::G_SUCCESS)
				{
					result = socket.Send(buffer, 256, bytesSent);
					if (result == GResult::G_SUCCESS)
					{
						std::cout << "Attempting to send chunk of data..." << std::endl;
						Sleep(500);
					}
					else
					{
						std::cout << "Lost Connection to the Server!" << std::endl;
						break;
					}
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