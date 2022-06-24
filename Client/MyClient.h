#pragma once
#include <GNet/IncludeMe.h>

class MyClient : public Client
{
protected:
	bool ProcessPacket(std::shared_ptr<Packet> packet) override;
	void OnConnect() override;
	//void OnConnectFail() override;
	//void OnDisconnect(std::string reason) override;
};