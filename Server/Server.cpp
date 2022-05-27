#include "Server.h"
#include <iostream>

bool Server::Initialise(IPEndpoint ip)
{
	master_fd.clear();
	connections.clear();
								
	if (Network::Initialize())
	{
		std::cout << "Winsock api successfully initialized." << std::endl;

		listeningSocket = Socket(ip.GetIPVersion());
		if (listeningSocket.Create() == GResult::G_SUCCESS)
		{
			std::cout << "Socket successfully created." << std::endl;

			if (listeningSocket.Listen(ip) == GResult::G_SUCCESS)
			{
				std::cout << "Socket successfully listening." << std::endl;

				//Poll for active connection requests
				WSAPOLLFD listeningSocketFD = {};

				listeningSocketFD.fd = listeningSocket.GetHandle();
				listeningSocketFD.events = POLLRDNORM; //Checks if there are incoming connections from our listening socket
				listeningSocketFD.revents = 0;

				master_fd.push_back(listeningSocketFD);

				return true;
			}
			else
			{
				std::cerr << "Failed to listen." << std::endl;
			}
			listeningSocket.Close();
		}
		else
		{
			std::cerr << "Socket failed to create." << std::endl;
		}
	}
	return false;
}

void Server::Frame()
{
	copy_fd = master_fd;

	if (WSAPoll(copy_fd.data(), copy_fd.size(), 1) > 0)
	{
#pragma region Listener
		WSAPOLLFD &listeningSocketFD = copy_fd[0];
		if (listeningSocketFD.revents &POLLRDNORM)
		{
			Socket newConnectionSocket;
			IPEndpoint newConnectionEndpoint;

			if (listeningSocket.Accept(newConnectionSocket, &newConnectionEndpoint) == GResult::G_SUCCESS)
			{
				connections.emplace_back(TCPConnection(newConnectionSocket, newConnectionEndpoint));

				TCPConnection& acceptedConnection = connections.back();
				std::cout << acceptedConnection.ToString() << " > New Connection Accepted. " << std::endl;

				WSAPOLLFD newConnectionFD = {};

				newConnectionFD.fd = newConnectionSocket.GetHandle();
				newConnectionFD.events = POLLRDNORM;
				newConnectionFD.revents = 0;

				master_fd.push_back(newConnectionFD);
			}
			else
			{
				std::cerr << "Failed to accept new connection " << std::endl;
			}
		}
#pragma endregion Listening socket code

		for (int i = copy_fd.size() - 1; i >= 1; i--)
		{
			int connectionIndex = i - 1;
			TCPConnection& connection = connections[connectionIndex];

			if (copy_fd[i].revents & POLLERR) //if error occured on this socket
			{
				CloseConnection(connectionIndex, "Socket Error");
				continue;
			}

			if (copy_fd[i].revents & POLLHUP) //if poll hangup occured on this socket
			{
				CloseConnection(connectionIndex, "Poll Hangup");
				continue;
			}

			if (copy_fd[i].revents & POLLNVAL) //if invalid socket
			{
				CloseConnection(connectionIndex, "Invalid Socket");
				continue;
			}

			if (copy_fd[i].revents & POLLRDNORM) //if normal data can be read without blocking
			{
				int bytesRecieved = 0;

				if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize) //Process size
				{
					int offset = connection.pm_incoming.currentPacketExtrtactionOffset;
					int size = connection.pm_incoming.currentPacketSize;

					bytesRecieved = recv(copy_fd[i].fd, (char*)&connection.pm_incoming.currentPacketSize + offset, sizeof(uint16_t) - offset, 0);
				}
				else //Process packet
				{
					int offset = connection.pm_incoming.currentPacketExtrtactionOffset;
					int size = connection.pm_incoming.currentPacketSize;

					bytesRecieved = recv(copy_fd[i].fd, (char*)&connection.buffer + offset, connection.pm_incoming.currentPacketSize - offset, 0);
				}
				

				if (bytesRecieved == 0)
				{
					CloseConnection(connectionIndex, "Recv==0");
					continue;
				}

				if (bytesRecieved == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						CloseConnection(connectionIndex, "Recv<0");
						continue;
					}
				}

				if (bytesRecieved > 0)
				{
					connection.pm_incoming.currentPacketExtrtactionOffset += bytesRecieved;
					if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize)
					{
						if (connection.pm_incoming.currentPacketExtrtactionOffset == sizeof(uint16_t))
						{
							connection.pm_incoming.currentPacketSize = ntohs(connection.pm_incoming.currentPacketSize);
							if (connection.pm_incoming.currentPacketSize > GNet::g_MaxPacketSize)
							{
								CloseConnection(connectionIndex, "Packet size too large");
								continue;
							}
							connection.pm_incoming.currentPacketExtrtactionOffset = 0;
							connection.pm_incoming.currentTask = PacketManagerTask::ProcessPacketContents;
						}
					}
					else //Processing packet contents
					{
						if (connection.pm_incoming.currentPacketExtrtactionOffset == connection.pm_incoming.currentPacketSize)
						{
							std::shared_ptr<Packet> packet = std::make_shared<Packet>();

							packet->buffer.resize(connection.pm_incoming.currentPacketSize);
							memcpy(&packet->buffer[0], connection.buffer, connection.pm_incoming.currentPacketSize);

							connection.pm_incoming.Append(packet);

							connection.pm_incoming.currentPacketSize = 0;
							connection.pm_incoming.currentPacketExtrtactionOffset = 0;
							connection.pm_incoming.currentTask = PacketManagerTask::ProcessPacketSize;
						}
					}
				}
			}
		}

		for (int i = connections.size() - 1; i >= 0; i--)
		{
			while (connections[i].pm_incoming.HasPendingPackets())
			{
				std::shared_ptr<Packet> frontPacket = connections[i].pm_incoming.Retrieve();
				if (!ProcessPacket(frontPacket))
				{
					CloseConnection(i, "Failed to process incoming packet. ");
					break;
				}
				connections[i].pm_incoming.Pop();
			}
		}
	}
}

void Server::CloseConnection(int connectionIndex, std::string reason)
{
	TCPConnection& connection = connections[connectionIndex];
	std::cerr << "[ " << reason << " ] " << "Connection Lost > " << connection.ToString() << std::endl;

	master_fd.erase(master_fd.begin() + ( connectionIndex + 1));
	copy_fd.erase(copy_fd.begin() + ( connectionIndex + 1));

	connection.Close();
	connections.erase(connections.begin() + connectionIndex);
}

bool Server::ProcessPacket(std::shared_ptr<Packet> packet)
{
	std::string chatmessage;
	uint32_t arraysize = 0;

	switch (packet->GetPacketType())
	{
	case PacketType::PT_CHATMESSAGE:

		*packet >> chatmessage;
		std::cout << "Chat Message: " << chatmessage << std::endl;
		break;

	case PacketType::PT_INTEGERARRAY:

		*packet >> arraysize;
		std::cout << "Arraysize: " << arraysize << std::endl;

		for (uint32_t i = 0; i < arraysize; i++)
		{
			uint32_t digit;
			*packet >> digit;
			std::cout << "Value [" << i << "] : " << digit << std::endl;
		}
		break;

	default:
		std::cout << "Unrecognised packet type: " << packet->GetPacketType() << std::endl;
		return false;

	}
	return true;
}
