#include "UDPSocket.h"
#include <assert.h>

namespace GNet
{
	UDPSocket::UDPSocket( IPVersion ipversion, SocketHandle handle)
		: Socket(ipversion, handle)
	{
	}

	GResult UDPSocket::Create()
	{
		assert(ip_version == IPVersion::IPv4 || ip_version == IPVersion::IPv6);

		//if the socket has already been setup we throw an error
		if (handle != INVALID_SOCKET)
		{
			return GResult::G_GENERICERROR;
		}

		handle = socket((ip_version == IPVersion::IPv4) ? AF_INET : AF_INET6, SOCK_DGRAM, IPPROTO_UDP); //attempt to create a UDP socket

		if (handle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}

		if (ip_version == IPVersion::IPv6)
		{
			int yes = 1;
			if (setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&yes, sizeof(yes)) != 0)
			{
				printf("failed to set ipv6 only sockopt\n");
				return GResult::G_GENERICERROR;
			}
		}
		return GResult::G_SUCCESS;
	}

	GResult UDPSocket::Close()
	{
		if (handle != INVALID_SOCKET)
		{
			return GResult::G_GENERICERROR;
		}

		int result = closesocket(handle);

		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}

		handle = INVALID_SOCKET;
		return GResult::G_SUCCESS;
	}
}