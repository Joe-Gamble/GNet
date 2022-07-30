#pragma once
#include "TCPConnection.h"

using namespace GNet;

namespace GNet
{
	class Server
	{
	public:
		bool Initialise();
		void Frame();

	protected:
		virtual void OnConnect(TCPConnection& newConnection);
		virtual void OnDisconnect(TCPConnection& lostConnection, std::string reason);
		virtual bool ProcessPacket(std::shared_ptr<Packet> packet);

		void CloseConnection(int connectionIndex, std::string reason);

		Socket listeningSocket;

		//List of active connections 
		std::vector<TCPConnection> connections;

		//Master file descriptor set
		std::vector<WSAPOLLFD> master_fd;

		//Copy fd's so we dont corrupt orginal data
		std::vector<WSAPOLLFD> copy_fd;
	};
}

