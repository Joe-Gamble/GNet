#include "Socket.h"
#include <assert.h>

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
