//Client Code

#include "MyClient.h"
#include <iostream>

int main()
{
	if (Network::Initialize())
	{
		MyClient client;

		std::string connectionString;
		std::cin >> connectionString;

		if (client.Connect(connectionString))
		{
			while (client.IsConnected())
			{
				client.Frame();
			}
		}
	}
	
	Network::Shutdown();
	return 0;
}