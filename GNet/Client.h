#pragma once
#include "TCPConnection.h"
#include "UDPConnection.h"

namespace GNet
{
	class Client
	{
	public:
		~Client();

		bool Connect(const std::string& ip);
		
		bool IsConnected();
		bool Frame();

	protected:
		virtual bool ProcessPacket(std::shared_ptr<Packet> packet);
		virtual void OnConnect();
		virtual void OnConnectFail();
		virtual void OnDisconnect(std::string reason);

		void CloseConnection(std::string reason);

		TCPConnection connection;

	private:
		bool Connect(IPEndpoint ip);

		bool isConnected = false;
		WSAPOLLFD master_fd;
		WSAPOLLFD copy_fd;
	};

	class UDPClient
	{
	public:
		bool Connect(const std::string& ip);

		bool IsConnected();
		bool Frame();

	protected:
		virtual bool ProcessPacket(std::shared_ptr<Packet> packet);
		virtual void OnConnect();
		virtual void OnConnectFail();
		virtual void OnDisconnect(std::string reason);

		void CloseConnection(std::string reason);

		UDPConnection connection;

	private:
		bool Connect(IPEndpoint ip);

		bool isConnected = false;
		WSAPOLLFD master_fd;
		WSAPOLLFD copy_fd;
	};
}
