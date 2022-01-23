#pragma once
#include <GNet/IncludeMe.h>

using namespace GNet;

class Server
{
public:
	bool Initialise(IPEndpoint ip);
	void Frame();
private:
	Socket listeningSocket;

	//List of active connections 
	std::vector<TCPConnection> connections;

	//Master file descriptor set
	std::vector<WSAPOLLFD> master_fd;
};
