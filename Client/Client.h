#pragma once
#include <GNet/IncludeMe.h>

using namespace GNet;

class Client
{
public:
	bool Connect(IPEndpoint ip);
	bool IsConnected();
	bool Frame();
private:
	bool isConnected = false;
	Socket socket;
};
