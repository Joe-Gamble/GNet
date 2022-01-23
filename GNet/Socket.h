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

		GResult Create();
		GResult Close();

		GResult Bind(IPEndpoint endpoint);
		GResult Listen(IPEndpoint endpoint, int backlog = 5);
		GResult Accept(Socket& outSocket, IPEndpoint* endpoint = nullptr);
		GResult Connect(IPEndpoint endpoint);

		GResult Send(const void* data, int numberOfBytes, int& bytesSent);
		GResult Recv(void* destination, int numberOfBytes, int& bytesReceived);
		GResult SendAll(const void* data, int numberOfBytes);
		GResult RecvAll(void* destination, int numberOfBytes);

		GResult Send(Packet& packet);
		GResult Recv(Packet& packet);

		SocketHandle GetHandle();
		IPVersion GetIPVersion();
		GResult SetBlocking(bool isBlocking);

	private:
		IPVersion m_ipversion = IPVersion::IPv4;
		SocketHandle m_handle = INVALID_SOCKET;
		GResult SetSocketOption(SocketOption option, BOOL Value);
	};
}