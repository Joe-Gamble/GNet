#include "TCPSocket.h"
#include <assert.h>

namespace GNet
{
	TCPSocket::TCPSocket(IPVersion ipversion, SocketHandle handle)
		: Socket(ipversion, handle)
	{
	}

	GResult TCPSocket::Create()
	{
		assert(ip_version == IPVersion::IPv4 || ip_version == IPVersion::IPv6);

		//if the socket has already been setup we throw an error
		if (handle != INVALID_SOCKET)
		{
			return GResult::G_GENERICERROR;
		}

		handle = socket((ip_version == IPVersion::IPv4) ? PF_INET : PF_INET6, SOCK_STREAM, IPPROTO_TCP); //attempt to create a socket

		if (handle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}

		if (SetBlocking(false) != GResult::G_SUCCESS)
		{
			return GResult::G_SUCCESS;
		}

		if (SetSocketOption(SocketOption::TCP_NO_DELAY, TRUE) != GResult::G_SUCCESS)
		{
			return GResult::G_GENERICERROR;
		}

		return GResult::G_SUCCESS;
	}

	GResult TCPSocket::Close()
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

	GResult TCPSocket::Bind(IPEndpoint endpoint)
	{
		assert(ip_version == endpoint.GetIPVersion());

		if (ip_version == IPVersion::IPv4)
		{
			sockaddr_in addr = endpoint.GetSockaddrIPv4();
			int result = bind(handle, (sockaddr*)(&addr), sizeof(sockaddr_in));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return GResult::G_GENERICERROR;
			}

		}
		else
		{
			sockaddr_in6 addr6 = endpoint.GetSockaddrIPv6();
			int result = bind(handle, (sockaddr*)(&addr6), sizeof(sockaddr_in6));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return GResult::G_GENERICERROR;
			}
		}
		return GResult::G_SUCCESS;
	}

	GResult TCPSocket::Listen(IPEndpoint endpoint, int backlog)
	{
		if (ip_version == IPVersion::IPv6)
		{
			if (SetSocketOption(SocketOption::IPv6ONLY, FALSE) != GResult::G_SUCCESS)
			{
				return GResult::G_GENERICERROR;
			}
		}


		if (Bind(endpoint) != GResult::G_SUCCESS)
		{
			return GResult::G_GENERICERROR;
		}

		int result = listen(handle, backlog);

		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}

		return GResult::G_SUCCESS;
	}

	GResult TCPSocket::Accept(TCPSocket& outSocket, IPEndpoint* endpoint)
	{
		assert(ip_version == IPVersion::IPv4 || ip_version == IPVersion::IPv6);

		if (ip_version == IPVersion::IPv4)
		{
			sockaddr_in addr = {};
			int len = sizeof(addr);

			SocketHandle acceptedConnectionHandle = accept(handle, (sockaddr*)&addr, &len);

			if (acceptedConnectionHandle == INVALID_SOCKET)
			{
				int error = WSAGetLastError();
				return GResult::G_GENERICERROR;
			}

			if (endpoint != nullptr)
			{
				*endpoint = IPEndpoint((sockaddr*)&addr);
			}

			outSocket = TCPSocket(IPVersion::IPv4, acceptedConnectionHandle);
		}
		else
		{
			sockaddr_in6 addr6 = {};
			int len = sizeof(addr6);

			SocketHandle acceptedConnectionHandle = accept(handle, (sockaddr*)&addr6, &len);

			if (acceptedConnectionHandle == INVALID_SOCKET)
			{
				int error = WSAGetLastError();
				return GResult::G_GENERICERROR;
			}

			if (endpoint != nullptr)
			{
				*endpoint = IPEndpoint((sockaddr*)&addr6);
			}

			outSocket = TCPSocket(IPVersion::IPv6, acceptedConnectionHandle);
		}

		return GResult::G_SUCCESS;
	}

	GResult TCPSocket::Connect(IPEndpoint endpoint)
	{
		assert(ip_version == endpoint.GetIPVersion());

		int result = 0;

		if (ip_version == IPVersion::IPv4)
		{
			sockaddr_in addr = endpoint.GetSockaddrIPv4();
			result = connect(handle, (sockaddr*)&addr, sizeof(sockaddr_in));
		}
		else
		{
			sockaddr_in6 addr6 = endpoint.GetSockaddrIPv6();
			result = connect(handle, (sockaddr*)&addr6, sizeof(sockaddr_in6));
		}

		if (result != 0)
		{
			int error = WSAGetLastError();
			std::cout << error << std::endl;
			return GResult::G_GENERICERROR;
		}

		return GResult::G_SUCCESS;
	}

	GResult TCPSocket::SetBlocking(bool isBlocking)
	{
		unsigned long nonBlocking = 1;
		unsigned long blocking = 0;

		int result = ioctlsocket(handle, FIONBIO, isBlocking ? &blocking : &nonBlocking);

		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}
		return GResult::G_SUCCESS;
	}
}
