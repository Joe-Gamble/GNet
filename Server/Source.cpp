//Server Code

#include <GNet/IncludeMe.h>
#include <iostream>

using namespace GNet;

int main()
{
	if (Network::Initialize())
	{
		std::cout << "Winsock api successfully initialized." << std::endl;

		Socket socket;
		if (socket.Create() == GResult::G_SUCCESS)
		{
			std::cout << "Socket successfully created." << std::endl;

			int port = 4790;
			if (socket.Listen(IPEndpoint("0.0.0.0", port)) == GResult::G_SUCCESS)
			{
				std::cout << "Socket successfully listening on port "<< port << "." << std::endl;
				Socket new_connection;
				if (socket.Accept(new_connection) == GResult::G_SUCCESS)
				{
					std::cout << "New connection accepted." << std::endl;

					char buffer[256];
					int bytesReceived = 0;

					GResult result = GResult::G_SUCCESS;

					while (result == GResult::G_SUCCESS)
					{
						result = new_connection.Recv(buffer, 256, bytesReceived);

						if (result != GResult::G_SUCCESS)
						{
							std::cout << "Client Disconnected." << std::endl;
							break;
						}
						std::cout << buffer << std::endl;
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