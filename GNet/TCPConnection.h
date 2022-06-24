#pragma once
#include "Socket.h"
#include "PacketManager.h"

namespace GNet
{
	//Wrapper class for encapsulating sockets and their respective ipEndpoints
	class TCPConnection
	{
	public:
		TCPConnection(Socket socket, IPEndpoint endpoint);
		TCPConnection() :socket(Socket()) {}

		void Close();
		std::string ToString();
		Socket socket;
		char buffer[GNet::g_MaxPacketSize];

		PacketManager pm_incoming;
		PacketManager pm_outgoing;
	private:
		IPEndpoint endpoint;
		std::string stringRepresentation = "";
	};
}
