#include "Server.h"
#include <iostream>
#include "Network.h"

namespace GNet
{
#pragma region TCP Server

	bool Server::Initialise()
	{
		IPEndpoint ip = IPEndpoint();

		master_fd.clear();
		connections.clear();

		listeningSocket = TCPSocket(ip.GetIPVersion());
		if (listeningSocket.Create() == GResult::G_SUCCESS)
		{
			std::cout << "Socket successfully created." << std::endl;

			if (listeningSocket.Listen(ip) == GResult::G_SUCCESS)
			{
				printf("Socket successfully listening on port %d. \n", ip.GetPort());

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

		return false;
	}

	void Server::Frame()
	{
		for (int i = 0; i < connections.size(); i++)
		{
			if (connections[i].pm_outgoing.HasPendingPackets())
			{
				master_fd[i + 1].events = POLLRDNORM | POLLWRNORM; //Cpu usage optimization
			}
		}

		copy_fd = master_fd;

		if (WSAPoll(copy_fd.data(), copy_fd.size(), 1) > 0)
		{
#pragma region Listener
			WSAPOLLFD& listeningSocketFD = copy_fd[0];
			if (listeningSocketFD.revents & POLLRDNORM)
			{
				TCPSocket newConnectionSocket;
				IPEndpoint newConnectionEndpoint;

				if (listeningSocket.Accept(newConnectionSocket, &newConnectionEndpoint) == GResult::G_SUCCESS)
				{
					connections.emplace_back(TCPConnection(newConnectionSocket, newConnectionEndpoint));

					TCPConnection& acceptedConnection = connections.back();
					WSAPOLLFD newConnectionFD = {};

					newConnectionFD.fd = newConnectionSocket.GetHandle();
					newConnectionFD.events = POLLRDNORM;
					newConnectionFD.revents = 0;

					master_fd.push_back(newConnectionFD);
					
					OnConnect(acceptedConnection);
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

				if (copy_fd[i].revents & POLLWRNORM) // IF normal data can be read without blocking
				{
					auto& pm = connection.pm_outgoing;

					while (pm.HasPendingPackets())
					{
						if (pm.currentTask == PacketManagerTask::ProcessPacketSize)
						{
							pm.currentPacketSize = pm.Retrieve()->buffer.size();
							uint16_t bigEndianPacketSize = htons(pm.currentPacketSize);
							int bytesSent = send(copy_fd[i].fd, (char*)(&bigEndianPacketSize) + pm.currentPacketExtrtactionOffset, sizeof(uint16_t) - pm.currentPacketExtrtactionOffset, 0);

							if (bytesSent > 0)
							{
								pm.currentPacketExtrtactionOffset += bytesSent;
							}

							if (pm.currentPacketExtrtactionOffset == sizeof(uint16_t))
							{
								pm.currentPacketExtrtactionOffset = 0;
								pm.currentTask = PacketManagerTask::ProcessPacketContents;
							}
							else // if full packet was not send, break out of the loop for sending outgoing packets for this connection - we will try again the next time we're writing data
							{
								break;
							}
						}
						else // Sending packet contents
						{
							char* bufferPtr = &pm.Retrieve()->buffer[0];
							int bytesSent = send(copy_fd[i].fd, (char*)(bufferPtr)+pm.currentPacketExtrtactionOffset, pm.currentPacketSize - pm.currentPacketExtrtactionOffset, 0);

							if (bytesSent > 0)
							{
								pm.currentPacketExtrtactionOffset += bytesSent;
							}

							if (pm.currentPacketExtrtactionOffset == pm.currentPacketSize) // has the full packet been sent
							{
								pm.currentPacketExtrtactionOffset = 0;
								pm.currentTask = PacketManagerTask::ProcessPacketSize;

								pm.Pop(); //remove packet from queue after it's completely sent
							}
							else
							{
								break;
							}

						}
					}

					if (!pm.HasPendingPackets())
					{
						master_fd[i].events = POLLRDNORM; //Cpu usage optimization
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
	}

	void Server::OnConnect(TCPConnection& newConnection)
	{
		std::cout << newConnection.ToString() << " - New Connection Accepted." << std::endl;
	}

	void Server::OnDisconnect(TCPConnection& lostConnection, std::string reason)
	{
		std::cout << "[" << reason << "] Connection lost: " << lostConnection.ToString() << "." << std::endl;
	}

	void Server::CloseConnection(int connectionIndex, std::string reason)
	{
		TCPConnection& connection = connections[connectionIndex];
		OnDisconnect(connection, reason);

		master_fd.erase(master_fd.begin() + (connectionIndex + 1));
		copy_fd.erase(copy_fd.begin() + (connectionIndex + 1));

		connection.Close();
		connections.erase(connections.begin() + connectionIndex);
	}



	bool Server::ProcessPacket(std::shared_ptr<Packet> packet)
	{
		std::cout << "Packet Received with size: " << packet->buffer.size() << std::endl;
		return true;
	}

#pragma endregion

#pragma region UDP Server
	bool UDPServer::Initialise()
	{
		IPEndpoint ip = IPEndpoint();

		master_fd.clear();

		socket = std::make_unique<UDPSocket>(ip.GetIPVersion());

		if (socket->Create() == GResult::G_SUCCESS)
		{
			std::cout << "[SERVER] UDP Socket successfully created." << std::endl;

			if (socket->Bind(ip) == GResult::G_SUCCESS)
			{
				std::cout << "[SERVER] UDP Socket successfully bound." << std::endl;

				if (socket->SetBlocking(false) == GResult::G_SUCCESS)
				{
					return true;
				}
			}
		}
		else
		{
			std::cerr << "[SERVER] UDP Socket failed to create." << std::endl;
		}

		return false;
	}

	void UDPServer::Frame()
	{
	
	}

	void UDPServer::OnConnect(UDPConnection& newConnection)
	{
		std::cout << newConnection.ToString() << " - New Connection Accepted." << std::endl;
	}

	void UDPServer::OnDisconnect(UDPConnection& lostConnection, std::string reason)
	{
		std::cout << "[" << reason << "] Connection lost: " << lostConnection.ToString() << "." << std::endl;
	}

	void UDPServer::CloseConnection(int connectionIndex, std::string reason)
	{
		/*
		UDPConnection& connection = connections[connectionIndex];
		OnDisconnect(connection, reason);

		master_fd.erase(master_fd.begin() + (connectionIndex + 1));
		copy_fd.erase(copy_fd.begin() + (connectionIndex + 1));

		connection.Close();
		connections.erase(connections.begin() + connectionIndex);
		*/
	}

	bool UDPServer::ProcessPacket(std::shared_ptr<Packet> packet)
	{
		std::cout << "Packet Received with size: " << packet->buffer.size() << std::endl;
		return true;
	}
#pragma endregion

}


