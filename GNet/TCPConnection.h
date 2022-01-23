#pragma once
#include "Socket.h"

namespace GNet
{
	//Wrapper class for encapsulating sockets and their respective ipEndpoints
	class TCPConnection
	{
	public:
		TCPConnection(Socket socket, IPEndpoint endpoint);
		void Close();
		std::string ToString();
		Socket socket;
	private:
		IPEndpoint endpoint;
		std::string stringRepresentation = "";
	};
}
