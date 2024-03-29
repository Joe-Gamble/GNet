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
		//Server endpoint constructor
		IPEndpoint(unsigned short port = 6112);

		//Client endpoint constructor
		IPEndpoint(const char* ip, unsigned short port = 6112);
		IPEndpoint(sockaddr* addr);

		IPVersion GetIPVersion();
		std::string GetHostname();
		std::string GetIPString();
		std::vector<uint8_t> GetIPBytes();

		unsigned short GetPort();
		sockaddr_in GetSockaddrIPv4();
		sockaddr_in6 GetSockaddrIPv6();
		void Print();

	private:
		bool GetPublicIPFromDomain(std::string url, std::string& websiteHTML, char* buffer);

		IPVersion ipversion = IPVersion::Unknown;
		std::string hostname = "";
		std::string ip_string = "";
		std::vector<uint8_t> ip_bytes;

		unsigned short port = 0;
	};
}
