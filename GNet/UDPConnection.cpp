#include "UDPConnection.h"

GNet::UDPConnection::UDPConnection(UDPSocket socket, IPEndpoint endpoint)
	:socket(socket), endpoint(endpoint)
{
	stringRepresentation = "[" + endpoint.GetIPString() + " - " + std::to_string(endpoint.GetPort()) + "]";
}

void GNet::UDPConnection::Close()
{
	socket.Close();
}

std::string GNet::UDPConnection::ToString()
{
	return stringRepresentation;
}
