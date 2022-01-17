#include "Socket.h"
#include <assert.h>
#include <iostream>

namespace GNet
{
	GNet::Socket::Socket(IPVersion ipversion, SocketHandle handle)
		: m_ipversion(ipversion), m_handle(handle)
	{
		assert(ipversion == IPVersion::IPv4);
	}

	GResult GNet::Socket::Create()
	{
		assert(m_ipversion == IPVersion::IPv4);

		//if the socket has already been setup we throw an error
		if (m_handle != INVALID_SOCKET)
		{
			return GResult::G_NOTYETIMPLEMENTED;
		}

		m_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //attempt to create a socket

		if (m_handle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return GResult::G_NOTYETIMPLEMENTED;
		}

		if (SetSocketOption(SocketOption::TCP_NO_DELAY, TRUE) != GResult::G_SUCCESS)
		{
			return GResult::G_NOTYETIMPLEMENTED;
		}

		return GResult::G_SUCCESS;
	}

	GResult GNet::Socket::Close()
	{
		if (m_handle != INVALID_SOCKET)
		{
			return GResult::G_NOTYETIMPLEMENTED;
		}

		int result = closesocket(m_handle);

		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_NOTYETIMPLEMENTED;
		}

		m_handle = INVALID_SOCKET;
		return GResult::G_SUCCESS;
	}

	GResult GNet::Socket::Bind(IPEndpoint endpoint)
	{
		sockaddr_in addr = endpoint.GetSockaddrIPv4();
		int result = bind(m_handle, (sockaddr*)(&addr), sizeof(sockaddr_in));
		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_NOTYETIMPLEMENTED;
		}
		return GResult::G_SUCCESS;
	}

	GResult GNet::Socket::Listen(IPEndpoint endpoint, int backlog)
	{
		if (Bind(endpoint) != GResult::G_SUCCESS)
		{
			return GResult::G_NOTYETIMPLEMENTED;
		}

		int result = listen(m_handle, backlog);

		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_NOTYETIMPLEMENTED;
		}

		return GResult::G_SUCCESS;
	}

	GResult GNet::Socket::Accept(Socket& outSocket)
	{
		sockaddr_in addr = {};
		int len = sizeof(addr);

		SocketHandle acceptedConnectionHandle = accept(m_handle, (sockaddr*)&addr, &len);

		if (acceptedConnectionHandle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return GResult::G_NOTYETIMPLEMENTED;
		}

		IPEndpoint newConnectionEndPoint((sockaddr*)&addr);
		std::cout << "New connection accepted!" << std::endl;

		newConnectionEndPoint.Print();

		outSocket = Socket(IPVersion::IPv4, acceptedConnectionHandle);
		return GResult::G_SUCCESS;
	}

	GResult GNet::Socket::Connect(IPEndpoint endpoint)
	{
		sockaddr_in addr = endpoint.GetSockaddrIPv4();
		int result = connect(m_handle, (sockaddr*)&addr, sizeof(sockaddr_in));

		if (result != 0)
		{
			return GResult::G_NOTYETIMPLEMENTED;
		}

		IPEndpoint newConnectionEndPoint((sockaddr*)&addr);
		std::cout << "New connection accepted!" << std::endl;

		newConnectionEndPoint.Print();

		return GResult::G_SUCCESS;
	}

	GResult GNet::Socket::Send(void* data, int numberOfBytes, int& bytesSent)
	{
		bytesSent = send(m_handle, (const char*)(data), numberOfBytes, NULL);

		if (bytesSent == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return GResult::G_NOTYETIMPLEMENTED;
		}
		return GResult::G_SUCCESS;
	}

	GResult GNet::Socket::Recv(void* destination, int numberOfBytes, int& bytesReceived)
	{
		bytesReceived = recv(m_handle, (char*)destination, numberOfBytes, NULL);

		if (bytesReceived == 0) //If Connection was perfectly closed
		{
			return GResult::G_NOTYETIMPLEMENTED;
		}
		else if (bytesReceived == SOCKET_ERROR) //Error receiving bytes
		{
			int error = WSAGetLastError();
			return GResult::G_NOTYETIMPLEMENTED;
		}
		return GResult::G_SUCCESS;
	}


	SocketHandle GNet::Socket::GetHandle()
	{
		return m_handle;
	}

	IPVersion GNet::Socket::GetIPVersion()
	{
		return m_ipversion;
	}

	GResult GNet::Socket::SetSocketOption(SocketOption option, BOOL value)
	{
		int result = 0;
		switch (option)
		{
		case SocketOption::TCP_NO_DELAY:
			{
			result = setsockopt(m_handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
				break;
			}
		default:
			return GResult::G_NOTYETIMPLEMENTED;
		}

		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_NOTYETIMPLEMENTED;
		}
		return GResult::G_SUCCESS;
	}
}
