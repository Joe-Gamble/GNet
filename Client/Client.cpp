#include "Client.h"
#include <iostream>

bool Client::Connect(IPEndpoint ip)
{
	isConnected = false;

	if (Network::Initialize())
	{
		std::cout << "Winsock api successfully initialized." << std::endl;

		socket = Socket(ip.GetIPVersion());
		//Create the socket
		if (socket.Create() == GResult::G_SUCCESS)
		{
			//Set client to use blocking socket whilst server is under development

			if (socket.SetBlocking(true) != GResult::G_SUCCESS)
				return false;

			std::cout << "Socket successfully created." << std::endl;

			//Attempt to connect to a socket listening on Port 4790
			if (socket.Connect(ip) == GResult::G_SUCCESS)
			{
				isConnected = true;
				std::cout << "Successfully connected to Server!" << std::endl;
				return true;
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
	return false;
}

bool Client::IsConnected()
{
	return isConnected;
}

bool Client::Frame()
{
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
		isConnected = false;
		return false;
	}

	std::cout << "Attempting to send chunk of data" << std::endl;
	Sleep(500);
	return true;
}