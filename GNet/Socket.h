#pragma once
#include "SocketHandler.h"
#include "PResult.h"
#include "IPVersion.h"
#include "SocketOption.h"
#include "IPEndpoint.h"

namespace GNet
{
	class Socket
	{
	public:
		Socket(IPVersion ipversion = IPVersion::IPv4, 
			SocketHandle handle = INVALID_SOCKET);

		GResult Create();
		GResult Close();

		SocketHandle GetHandle();
		IPVersion GetIPVersion();
	private:
		IPVersion m_ipversion = IPVersion::IPv4;
		SocketHandle m_handle = INVALID_SOCKET;
		GResult SetSocketOption(SocketOption option, BOOL Value);
	};
}