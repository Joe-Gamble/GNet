#pragma once
#include "TCPConnection.h"
#include "UDPConnection.h"
#include "ServerClientData.h"

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

		TCPSocket listeningSocket;

		//List of active connections 
		std::vector<TCPConnection> connections;

		//Master file descriptor set
		std::vector<WSAPOLLFD> master_fd;

		//Copy fd's so we dont corrupt orginal data
		std::vector<WSAPOLLFD> copy_fd;
	};

	class UDPServer
	{
	public:
		bool Initialise();
		void Frame();

	protected:
		virtual void OnConnect(UDPConnection& newConnection);
		virtual void OnDisconnect(UDPConnection& lostConnection, std::string reason);
		virtual bool ProcessPacket(std::shared_ptr<Packet> packet);

		void CloseConnection(int connectionIndex, std::string reason);

		std::unique_ptr<UDPSocket> socket;

		//Total active clients
		int numberOfClients = 0;

		bool clientConnectedStates[g_MaxClients];
		IPEndpoint clientEndpoints[g_MaxClients];
		ServerClientData clientData[g_MaxClients];

		//Master file descriptor set
		std::vector<WSAPOLLFD> master_fd;

		//Copy fd's so we dont corrupt orginal data
		std::vector<WSAPOLLFD> copy_fd;
	};
}

