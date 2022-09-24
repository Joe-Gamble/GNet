#pragma once
#include "TCPSocket.h"
#include "PacketManager.h"

namespace GNet
{
	//Wrapper class for encapsulating sockets and their respective ipEndpoints
	class TCPConnection
	{
	public:
		TCPConnection(TCPSocket socket, IPEndpoint endpoint);
		TCPConnection() :socket(TCPSocket()) {}

		void Close();
		std::string ToString();
		TCPSocket socket;
		char buffer[GNet::g_MaxPacketSize];

		PacketManager pm_incoming;
		PacketManager pm_outgoing;
	private:
		IPEndpoint endpoint;
		std::string stringRepresentation = "";
	};
}
