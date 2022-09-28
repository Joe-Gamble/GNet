#include "Socket.h"

namespace GNet
{
	class UDPSocket : public Socket
	{
	public:
		UDPSocket(IPVersion ipversion = IPVersion::IPv4,
			SocketHandle handle = INVALID_SOCKET);

		GResult Create() override;
		GResult Close() override;
	};
}