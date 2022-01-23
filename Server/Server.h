#pragma once
#include <GNet/IncludeMe.h>

using namespace GNet;

class Server
{
public:
	bool Initialise(IPEndpoint ip);
	void Frame();
private:
	void CloseConnection(int connectionIndex, std::string reason);
	Socket listeningSocket;

	//List of active connections 
	std::vector<TCPConnection> connections;

	//Master file descriptor set
	std::vector<WSAPOLLFD> master_fd;

	//Copy fd's so we dont corrupt orginal data
	std::vector<WSAPOLLFD> copy_fd;
};
