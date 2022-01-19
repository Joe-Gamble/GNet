#include "Socket.h"
#include <assert.h>
#include <iostream>

namespace GNet
{
	Socket::Socket(IPVersion ipversion, SocketHandle handle)
		: m_ipversion(ipversion), m_handle(handle)
	{
		assert(ipversion == IPVersion::IPv4);
	}

	GResult Socket::Create()
	{
		assert(m_ipversion == IPVersion::IPv4);

		//if the socket has already been setup we throw an error
		if (m_handle != INVALID_SOCKET)
		{
			return GResult::G_GENERICERROR;
		}

		m_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //attempt to create a socket

		if (m_handle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
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
		sockaddr_in addr = endpoint.GetSockaddrIPv4();
		int result = bind(m_handle, (sockaddr*)(&addr), sizeof(sockaddr_in));
		if (result != 0)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}
		return GResult::G_SUCCESS;
	}

	GResult Socket::Listen(IPEndpoint endpoint, int backlog)
	{
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

	GResult Socket::Accept(Socket& outSocket)
	{
		sockaddr_in addr = {};
		int len = sizeof(addr);

		SocketHandle acceptedConnectionHandle = accept(m_handle, (sockaddr*)&addr, &len);

		if (acceptedConnectionHandle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}

		IPEndpoint newConnectionEndPoint((sockaddr*)&addr);
		std::cout << "New connection accepted!" << std::endl;

		newConnectionEndPoint.Print();

		outSocket = Socket(IPVersion::IPv4, acceptedConnectionHandle);
		return GResult::G_SUCCESS;
	}

	GResult Socket::Connect(IPEndpoint endpoint)
	{
		sockaddr_in addr = endpoint.GetSockaddrIPv4();
		int result = connect(m_handle, (sockaddr*)&addr, sizeof(sockaddr_in));

		if (result != 0)
		{
			return GResult::G_GENERICERROR;
		}

		IPEndpoint newConnectionEndPoint((sockaddr*)&addr);
		std::cout << "New connection accepted!" << std::endl;

		newConnectionEndPoint.Print();

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
		uint32_t encodedPacketSize = htonl(packet.buffer.size()); 
		
		GResult result = SendAll(&encodedPacketSize, sizeof(uint32_t)); //Send size of packet
		if (result != GResult::G_SUCCESS)
			return GResult::G_GENERICERROR;

		result = SendAll(packet.buffer.data(), sizeof(packet.buffer.size())); //Send packet data
		if (result != GResult::G_SUCCESS)
			return GResult::G_GENERICERROR;

		return GResult::G_SUCCESS;

	}
	GResult Socket::Recv(Packet& packet)
	{
		packet.Clear();

		uint32_t encodedSize = 0;
		GResult result = RecvAll(&encodedSize, sizeof(uint32_t));
		if (result != GResult::G_SUCCESS)
			return GResult::G_GENERICERROR;

		uint32_t bufferSize = ntohl(encodedSize);

		if (bufferSize > GNet::g_MaxPacketSize)
			return GResult::G_GENERICERROR;

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
}
