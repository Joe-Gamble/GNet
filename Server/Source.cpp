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

					uint32_t a(0), b(0), c(0);
					Packet packet;

					while (true)
					{
						GResult result = new_connection.Recv(packet);

						if (result != GResult::G_SUCCESS)
						{
							std::cerr << "Could not receive packet" << std::endl;
							break;
						}

						packet >> a >> b >> c;
						std::cout << a << "," << b << "," << c << std::endl;
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