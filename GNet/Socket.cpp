#include "Socket.h"
#include <assert.h>
#include <iostream>

namespace GNet
{
	Socket::Socket(IPVersion ipversion, SocketHandle handle)
		: m_ipversion(ipversion), m_handle(handle)
	{
		assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);
	}

	GResult Socket::Create()
	{
		assert(m_ipversion == IPVersion::IPv4 || m_ipversion == IPVersion::IPv6);

		//if the socket has already been setup we throw an error
		if (m_handle != INVALID_SOCKET)
		{
			return GResult::G_GENERICERROR;
		}

		m_handle = socket((m_ipversion == IPVersion::IPv4) ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP); //attempt to create a socket

		if (m_handle == INVALID_SOCKET)
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

	GResult Socket::Close()
	{
		if (m_handle != INVALID_SOCKET)
		{
			return GResult::G_GENERICERROR;
		}

		int result = closesocket(m_handle);

		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}

		m_handle = INVALID_SOCKET;
		return GResult::G_SUCCESS;
	}

	GResult Socket::Bind(IPEndpoint endpoint)
	{
		assert(m_ipversion == endpoint.GetIPVersion());

		if (m_ipversion == IPVersion::IPv4)
		{
			sockaddr_in addr = endpoint.GetSockaddrIPv4();
			int result = bind(m_handle, (sockaddr*)(&addr), sizeof(sockaddr_in));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return GResult::G_GENERICERROR;
			}
			
		}
		else
		{
			sockaddr_in6 addr6 = endpoint.GetSockaddrIPv6();
			int result = bind(m_handle, (sockaddr*)(&addr6), sizeof(sockaddr_in6));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return GResult::G_GENERICERROR;
			}
		}
		return GResult::G_SUCCESS;
	}

	GResult Socket::Listen(IPEndpoint endpoint, int backlog)
	{
		if (m_ipversion == IPVersion::IPv6)
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

		int result = listen(m_handle, backlog);

		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}

		return GResult::G_SUCCESS;
	}

	GResult Socket::Accept(Socket& outSocket, IPEndpoint* endpoint)
	{
		assert(m_ipversion == IPVersion::IPv4 || m_ipversion == IPVersion::IPv6);

		if (m_ipversion == IPVersion::IPv4)
		{
			sockaddr_in addr = {};
			int len = sizeof(addr);

			SocketHandle acceptedConnectionHandle = accept(m_handle, (sockaddr*)&addr, &len);

			if (acceptedConnectionHandle == INVALID_SOCKET)
			{
				int error = WSAGetLastError();
				return GResult::G_GENERICERROR;
			}

			if (endpoint != nullptr)
			{
				*endpoint = IPEndpoint((sockaddr*)&addr);
			}

			outSocket = Socket(IPVersion::IPv4, acceptedConnectionHandle);
		}
		else
		{
			sockaddr_in6 addr6 = {};
			int len = sizeof(addr6);

			SocketHandle acceptedConnectionHandle = accept(m_handle, (sockaddr*)&addr6, &len);

			if (acceptedConnectionHandle == INVALID_SOCKET)
			{
				int error = WSAGetLastError();
				return GResult::G_GENERICERROR;
			}

			if (endpoint != nullptr)
			{
				*endpoint = IPEndpoint((sockaddr*)&addr6);
			}

			outSocket = Socket(IPVersion::IPv6, acceptedConnectionHandle);
		}
		
		return GResult::G_SUCCESS;
	}

	GResult Socket::Connect(IPEndpoint endpoint)
	{
		assert(m_ipversion == endpoint.GetIPVersion());

		int result = 0;

		if (m_ipversion == IPVersion::IPv4)
		{
			sockaddr_in addr = endpoint.GetSockaddrIPv4();
			result = connect(m_handle, (sockaddr*)&addr, sizeof(sockaddr_in));
		}
		else
		{
			sockaddr_in6 addr6 = endpoint.GetSockaddrIPv6();
			result = connect(m_handle, (sockaddr*)&addr6, sizeof(sockaddr_in6));
		}

		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}

		return GResult::G_SUCCESS;
	}

	GResult Socket::Send(const void* data, int numberOfBytes, int& bytesSent)
	{
		bytesSent = send(m_handle, (const char*)(data), numberOfBytes, NULL);

		if (bytesSent == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}
		return GResult::G_SUCCESS;
	}

	GResult Socket::Recv(void* destination, int numberOfBytes, int& bytesReceived)
	{
		bytesReceived = recv(m_handle, (char*)destination, numberOfBytes, NULL);

		if (bytesReceived == 0) //If Connection was perfectly closed
		{
			return GResult::G_GENERICERROR;
		}
		else if (bytesReceived == SOCKET_ERROR) //Error receiving bytes
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}
		return GResult::G_SUCCESS;
	}

	GResult Socket::SendAll(const void* data, int numberOfBytes)
	{
		int totalBytesSent = 0;

		while (totalBytesSent < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesSent;
			int bytesSent = 0;

			char* bufferOffset = (char*)data + totalBytesSent;
			 
			GResult result = Send(bufferOffset, bytesRemaining, bytesSent);

			if (result != GResult::G_SUCCESS)
			{
				return GResult::G_GENERICERROR; 
			}
			totalBytesSent += bytesSent;
		}
		return GResult::G_SUCCESS;
	}

	GResult Socket::RecvAll(void* data, int numberOfBytes)
	{
		int totalBytesReceived = 0;

		while (totalBytesReceived < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesReceived;
			int bytesSent = 0;

			char* bufferOffset = (char*)data + totalBytesReceived;

			GResult result = Recv(bufferOffset, bytesRemaining, bytesSent);

			if (result != GResult::G_SUCCESS)
			{
				return GResult::G_GENERICERROR;
			}
			totalBytesReceived += bytesSent;
		}
		return GResult::G_SUCCESS;
	}

	GResult Socket::Send(Packet& packet)
	{
		//SEND PACKET SIZE
		uint16_t encodedPacketSize = htons(packet.buffer.size()); 
		GResult result = SendAll(&encodedPacketSize, sizeof(uint16_t)); //Send size of packet
		if (result != GResult::G_SUCCESS)
			return GResult::G_GENERICERROR;

		//SEND PACKET
		result = SendAll(packet.buffer.data(), packet.buffer.size()); //Send packet data
		if (result != GResult::G_SUCCESS)
			return GResult::G_GENERICERROR;

		return GResult::G_SUCCESS;
	}
	
	GResult Socket::Recv(Packet& packet)
	{
		packet.Clear();

		//RECEIVE PACKET SIZE
		uint16_t encodedSize = 0;
		GResult result = RecvAll(&encodedSize, sizeof(uint16_t));
		if (result != GResult::G_SUCCESS)
			return GResult::G_GENERICERROR;

		uint16_t bufferSize = ntohs(encodedSize);

		if (bufferSize > GNet::g_MaxPacketSize)
			return GResult::G_GENERICERROR;

		//RECEIVE PACKET CONTENTS
		packet.buffer.resize(bufferSize); //resize so packet buffer can store the data we want
		result = RecvAll(&packet.buffer[0], bufferSize);
		if (result != GResult::G_SUCCESS)
			return GResult::G_GENERICERROR;

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
		case SocketOption::IPv6ONLY:
		{
			result = setsockopt(m_handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(value));
			break;
		}
		default:
			return GResult::G_GENERICERROR;
		}

		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}
		return GResult::G_SUCCESS;
	}

	GResult Socket::SetBlocking(bool isBlocking)
	{
		unsigned long nonBlocking = 1;
		unsigned long blocking = 0;
		
		int result = ioctlsocket(m_handle, FIONBIO, isBlocking ? &blocking : &nonBlocking);

		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}
		return GResult::G_SUCCESS;
	}
}
