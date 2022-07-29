#pragma once
#include "IPVersion.h"
#include <string>
#include <vector>
#include <WS2tcpip.h>

namespace GNet
{
	class IPEndpoint
	{
	public:
		IPEndpoint() {}
		IPEndpoint(unsigned short port);
		IPEndpoint(const char* ip, unsigned short port);
		IPEndpoint(sockaddr* addr);

		IPVersion GetIPVersion();
		std::string GetHostname();
		std::string GetIPString();
		void GetWebsite(std::string url, std::string& websiteHTML, char* buffer);
		std::vector<uint8_t> GetIPBytes();

		unsigned short GetPort();
		sockaddr_in GetSockaddrIPv4();
		sockaddr_in6 GetSockaddrIPv6();
		void Print();

	private:
		IPVersion ipversion = IPVersion::Unknown;
		std::string hostname = "";
		std::string ip_string = "";
		std::vector<uint8_t> ip_bytes;

		unsigned short port = 0;
	};
}
