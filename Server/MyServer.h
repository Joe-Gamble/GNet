#pragma
#include <GNet/IncludeMe.h>

using namespace GNet;

class MyServer : public Server
{
private:
	void OnConnect(TCPConnection& newConnection) override;
	void OnDisconnect(TCPConnection& lostConnection, const std::string& reason) override;
	bool ProcessPacket(std::shared_ptr<Packet> packet, int connectiojnIndex) override;
};