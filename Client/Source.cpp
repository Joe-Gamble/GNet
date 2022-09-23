//Client Code

#include "MyClient.h"
#include <iostream>

int main()
{
	if (Network::Initialize())
	{
		MyClient client;

		const std::string string = "192.168.0.23";

		if (client.Connect(string))
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