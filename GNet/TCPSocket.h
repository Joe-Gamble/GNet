#pragma once
#include "SocketHandler.h"
#include "GResult.h"
#include "IPVersion.h"
#include "SocketOption.h"
#include "IPEndpoint.h"
#include "Constants.h"
#include "Packet.h"
#include "Socket.h"

namespace GNet
{
	class TCPSocket : public Socket
	{
	public:
		TCPSocket(IPVersion ipversion = IPVersion::IPv4,
			SocketHandle handle = INVALID_SOCKET);

		GResult Create() override;
		GResult Close() override;

		GResult Listen(IPEndpoint endpoint, int backlog = 5);
		GResult Accept(TCPSocket& outSocket, IPEndpoint* endpoint = nullptr);
		GResult Connect(IPEndpoint endpoint);
	};
}