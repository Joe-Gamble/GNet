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
	std::vector<WSAPOLLFD> copy_fd = master_fd; //Copy fd's so we dont corrupt orginal data

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

		for (int i = 1; i < copy_fd.size(); i++)
		{
			int connectionIndex = i - 1;
			TCPConnection connection = connections[connectionIndex];

			if (copy_fd[i].revents & POLLERR) //if error occured on this socket
			{
				std::cerr << "Poll error ocurred on connection > " << connection.ToString() << std::endl;

				master_fd.erase(master_fd.begin() + i);
				copy_fd.erase(copy_fd.begin() + i);
				connection.Close();
				connections.erase(connections.begin() + connectionIndex);
				i -= 1;
				continue;
			}

			if (copy_fd[i].revents & POLLHUP) //if poll hangup occured on this socket
			{
				std::cerr << "Poll hangup ocurred on connection > " << connection.ToString() << std::endl;

				master_fd.erase(master_fd.begin() + i);
				copy_fd.erase(copy_fd.begin() + i);
				connection.Close();
				connections.erase(connections.begin() + connectionIndex);
				i -= 1;
				continue;
			}

			if (copy_fd[i].revents & POLLNVAL) //if invalid socket
			{
				std::cerr << "Poll hangup ocurred on connection > " << connection.ToString() << std::endl;

				master_fd.erase(master_fd.begin() + i);
				copy_fd.erase(copy_fd.begin() + i);
				connection.Close();
				connections.erase(connections.begin() + connectionIndex);
				i -= 1;
				continue;
			}

			if (copy_fd[i].revents & POLLRDNORM) //if normal data can be read without blocking
			{
				char buffer[GNet::g_MaxPacketSize];

				int bytesRecieved = 0;
				bytesRecieved = recv(copy_fd[i].fd, buffer, GNet::g_MaxPacketSize, 0);

				if (bytesRecieved == 0)
				{
					std::cerr << "[Recv==0] Connection lost on > " << connection.ToString() << std::endl;

					master_fd.erase(master_fd.begin() + i);
					copy_fd.erase(copy_fd.begin() + i);	
					connection.Close();
					connections.erase(connections.begin() + connectionIndex);
					i -= 1;
					continue;
				}

				if (bytesRecieved == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						std::cerr << "[Recv<0] Connection lost on > " << connection.ToString() << std::endl;

						master_fd.erase(master_fd.begin() + i);
						copy_fd.erase(copy_fd.begin() + i);
						connection.Close();
						connections.erase(connections.begin() + connectionIndex);
						i -= 1;
						continue;
					}
				}

				if (bytesRecieved > 0)
				{
					std::cout << connection.ToString() << "< Message Size: " << bytesRecieved << "." << std::endl;
				}
			}
		}
	}
}	