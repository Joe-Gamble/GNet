#pragma once
#include "SocketHandler.h"
#include "GResult.h"
#include "IPVersion.h"
#include "SocketOption.h"
#include "IPEndpoint.h"
#include "Constants.h"
#include "Packet.h"

namespace GNet
{
	class Socket
	{
	public:
		Socket(IPVersion ipversion = IPVersion::IPv4, 
			SocketHandle handle = INVALID_SOCKET);

		virtual GResult Create() = 0;
		virtual GResult Close() = 0;

		GResult Send(const void* data, int numberOfBytes, int& bytesSent);
		GResult Recv(void* destination, int numberOfBytes, int& bytesReceived);
		GResult SendAll(const void* data, int numberOfBytes);
		GResult RecvAll(void* destination, int numberOfBytes);

		GResult Send(Packet& packet);
		GResult Recv(Packet& packet);

		SocketHandle GetHandle();
		IPVersion GetIPVersion();

	protected:
		IPVersion ip_version = IPVersion::IPv4;
		SocketHandle handle = INVALID_SOCKET;
		GResult SetSocketOption(SocketOption option, BOOL Value);
	};
}