#include "TCPConnection.h"

using namespace GNet;

TCPConnection::TCPConnection(Socket socket, IPEndpoint endpoint)
	:socket(socket), endpoint(endpoint)
{
	stringRepresentation = "[" + endpoint.GetIPString() + " - " + std::to_string(endpoint.GetPort()) + "]";
}

void TCPConnection::Close()
{
	socket.Close();
}

std::string GNet::TCPConnection::ToString()
{
	return stringRepresentation;
}