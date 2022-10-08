#include "Socket.h"
#include <assert.h>
#include <iostream>

namespace GNet
{
	Socket::Socket(IPVersion ipversion, SocketHandle handle)
		: ip_version(ipversion), handle(handle)
	{
		assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);
	}

	GResult Socket::Bind(IPEndpoint endpoint)
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

	GResult Socket::Send(const void* data, int numberOfBytes, int& bytesSent)
	{
		bytesSent = send(handle, (const char*)(data), numberOfBytes, NULL);

		if (bytesSent == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}
		return GResult::G_SUCCESS;
	}

	GResult Socket::Recv(void* destination, int numberOfBytes, int& bytesReceived)
	{
		bytesReceived = recv(handle, (char*)destination, numberOfBytes, NULL);

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

	SocketHandle& GNet::Socket::GetHandle()
	{
		return handle;
	}

	IPVersion& GNet::Socket::GetIPVersion()
	{
		return ip_version;
	}

	GResult GNet::Socket::SetSocketOption(SocketOption option, IPVersion version)
	{
		int result = 0;
		int yes = 1;

		switch (option)
		{
			case SocketOption::TCP_NO_DELAY:
			{
				result = setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&yes, sizeof(yes));
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

		if (ip_version == IPVersion::IPv6)
		{
			result = setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&yes, sizeof(yes));
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

		if (ioctlsocket(handle, FIONBIO, isBlocking ? &blocking : &nonBlocking) == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return GResult::G_GENERICERROR;
		}
		return GResult::G_SUCCESS;
	}
}
