#include "Client.h"

namespace GNet
{
	bool Client::Connect(const std::string& ip)
	{
		return Connect(IPEndpoint(ip.c_str()));
	}

	bool Client::Connect(IPEndpoint ip)
	{
		isConnected = false;

		Socket socket = Socket(ip.GetIPVersion());
		//Create the socket
		if (socket.Create() == GResult::G_SUCCESS)
		{
			//Set client to use blocking socket whilst server is under development

			if (socket.SetBlocking(true) != GResult::G_SUCCESS)
				return false;

			std::cout << "Socket successfully created." << std::endl;

			//Attempt to connect to a socket listening on Port 6112
			if (socket.Connect(ip) == GResult::G_SUCCESS)
			{
				if (socket.SetBlocking(false) == GResult::G_SUCCESS)
				{
					connection = TCPConnection(socket, ip);
					master_fd.fd = connection.socket.GetHandle();
					master_fd.events = POLLRDNORM;
					master_fd.revents = 0;
					isConnected = true;

					OnConnect();
					return true;
				}
			}
			else
			{
				printf("Socket failed to connect to %s with port %hu.", ip.GetIPString(), ip.GetPort());
			}
			socket.Close();
		}
		else
		{
			std::cerr << "Socket failed to create." << std::endl;
		}

		OnConnectFail();
		return false;
 
	}

	bool Client::IsConnected()
	{
		return isConnected;
	}

	bool Client::Frame()
	{
		if (connection.pm_outgoing.HasPendingPackets())
		{
			master_fd.events = POLLRDNORM | POLLWRNORM; //cpu optimization
		}

		copy_fd = master_fd;

		if (WSAPoll(&copy_fd, 1, 1) > 0)
		{

			if (copy_fd.revents & POLLERR) //if error occured on this socket
			{
				CloseConnection("Socket Error");
				return false;
			}

			if (copy_fd.revents & POLLHUP) //if poll hangup occured on this socket
			{
				CloseConnection("Poll Hangup");
				return false;
			}

			if (copy_fd.revents & POLLNVAL) //if invalid socket 
			{
				CloseConnection("Invalid Socket");
				return false;
			}

			if (copy_fd.revents & POLLRDNORM) //if normal data can be read without blocking
			{
				int bytesRecieved = 0;

				if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize) //Process size
				{
					int offset = connection.pm_incoming.currentPacketExtrtactionOffset;
					int size = connection.pm_incoming.currentPacketSize;

					bytesRecieved = recv(copy_fd.fd, (char*)&connection.pm_incoming.currentPacketSize + offset, sizeof(uint16_t) - offset, 0);
				}
				else //Process packet
				{
					int offset = connection.pm_incoming.currentPacketExtrtactionOffset;
					int size = connection.pm_incoming.currentPacketSize;

					bytesRecieved = recv(copy_fd.fd, (char*)&connection.buffer + offset, connection.pm_incoming.currentPacketSize - offset, 0);
				}


				if (bytesRecieved == 0)
				{
					CloseConnection("Recv==0");
					return false;
				}

				if (bytesRecieved == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						CloseConnection("Recv<0");
						return false;
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
								CloseConnection("Packet size too large");
								return false;
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

			if (copy_fd.revents & POLLWRNORM) // IF normal data can be read without blocking
			{
				auto& pm = connection.pm_outgoing;

				while (pm.HasPendingPackets())
				{
					if (pm.currentTask == PacketManagerTask::ProcessPacketSize)
					{
						pm.currentPacketSize = pm.Retrieve()->buffer.size();
						uint16_t bigEndianPacketSize = htons(pm.currentPacketSize);
						int bytesSent = send(copy_fd.fd, (char*)(&bigEndianPacketSize) + pm.currentPacketExtrtactionOffset, sizeof(uint16_t) - pm.currentPacketExtrtactionOffset, 0);

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
						int bytesSent = send(copy_fd.fd, (char*)(bufferPtr)+pm.currentPacketExtrtactionOffset, pm.currentPacketSize - pm.currentPacketExtrtactionOffset, 0);

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
					master_fd.events = POLLRDNORM; //Cpu usage optimization
				}
			}


			while (connection.pm_incoming.HasPendingPackets())
			{
				std::shared_ptr<Packet> frontPacket = connection.pm_incoming.Retrieve();
				if (!ProcessPacket(frontPacket))
				{
					CloseConnection("Failed to process incoming packet. ");
					return false;
				}
				connection.pm_incoming.Pop();
			}
		}
	}

	bool Client::ProcessPacket(std::shared_ptr<Packet> packet)
	{
		std::cout << "Packet Received with size: " << packet->buffer.size() << std::endl;
		return true;
	}

	void Client::OnConnect()
	{
		std::cout << "Successfully connected!" << std::endl;
	}

	void Client::OnConnectFail()
	{
		std::cout << "Failed to connect." << std::endl;
	}

	void Client::OnDisconnect(std::string reason)
	{
		std::cout << "Loct connection. Reason: " << reason << "." << std::endl;
	}
	void Client::CloseConnection(std::string reason)
	{
		OnDisconnect(reason);
		master_fd.fd = 0;
		isConnected = false;
		connection.Close();
	}
}

/*
#include "Client.h"
#include <iostream>

bool GNet::Client::Connect(IPEndpoint ip)
{
	isConnected = false;

	if (Network::Initialize())
	{
		std::cout << "Winsock api successfully initialized." << std::endl;

		socket = Socket(ip.GetIPVersion());
		//Create the socket
		if (socket.Create() == GResult::G_SUCCESS)
		{
			//Set client to use blocking socket whilst server is under development

			if (socket.SetBlocking(true) != GResult::G_SUCCESS)
				return false;

			std::cout << "Socket successfully created." << std::endl;

			//Attempt to connect to a socket listening on Port 4790
			if (socket.Connect(ip) == GResult::G_SUCCESS)
			{
				isConnected = true;
				std::cout << "Successfully connected to Server!" << std::endl;
				return true;
			}
			else
			{
				std::cerr << "Failed to connect to the Server" << std::endl;
			}
			socket.Close();
		}
		else
		{
			std::cerr << "Socket failed to create." << std::endl;
		}
	}
	return false;
}

bool GNet::Client::IsConnected()
{
	return isConnected;
}

bool GNet::Client::Frame()
{
	Packet incomingPacket;

	if (socket.Recv(incomingPacket) != GResult::G_SUCCESS)
	{
		std::cout << "Lost?" << std::endl;
		isConnected = false;
		return false;
	}

	if (!ProcessPacket(incomingPacket))
	{
		isConnected = false;
		return false;
	}
	return true;
}

bool GNet::Client::ProcessPacket(Packet& packet)
{
	std::string chatmessage;
	uint32_t arraysize = 0;

	switch (packet.GetPacketType())
	{
	case PacketType::PT_CHATMESSAGE:

		packet >> chatmessage;
		std::cout << "Chat Message: " << chatmessage << std::endl;
		break;

	case PacketType::PT_INTEGERARRAY:

		packet >> arraysize;
		std::cout << "Arraysize: " << arraysize << std::endl;

		for (uint32_t i = 0; i < arraysize; i++)
		{
			uint32_t digit;
			packet >> digit;
			std::cout << "Value [" << i << "] : " << digit << std::endl;
		}
		break;

	default:
		return false;

	}
	return true;
}
*/

