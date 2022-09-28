#pragma once
#include "UDPSocket.h"
#include "PacketManager.h"

namespace GNet
{
	class UDPConnection
	{
	public:
		UDPConnection(UDPSocket socket, IPEndpoint endpoint);
		UDPConnection() :socket(UDPSocket()) {}

		void Close();
		std::string ToString();
		UDPSocket socket;
		char buffer[GNet::g_MaxPacketSize];

		PacketManager pm_incoming;
		PacketManager pm_outgoing;
	private:
		IPEndpoint endpoint;
		std::string stringRepresentation = "";
	};
}