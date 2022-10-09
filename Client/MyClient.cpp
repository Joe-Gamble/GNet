#include "MyClient.h"

bool MyClient::ProcessPacket(std::shared_ptr<Packet> packet)
{
	std::string chatmessage;
	uint32_t arraysize = 0;

	switch (packet->GetPacketType())
	{
	default:
		return false;

	}
	return true;
}

void MyClient::OnConnect()
{
	std::cout << "Successfully connected to the server!" << std::endl;
}
