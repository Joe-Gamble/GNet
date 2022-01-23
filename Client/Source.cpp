//Client Code

#include "Client.h"
#include <iostream>

using namespace GNet;

int main()
{
	Client client;
	 
	if (client.Connect(IPEndpoint("::1", 6112)))
	{
		while (client.IsConnected())
		{
			client.Frame();
		}
	}
	Network::Shutdown();
	system("Pause");
	return 0;
}