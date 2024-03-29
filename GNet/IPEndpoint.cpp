#include "IPEndpoint.h"
#include <assert.h>
#include <windows.h>
#include <iostream>
#include <string>
#include "Helpers.h"

#include <vector>
#include <locale>
#include <sstream>
#include "Socket.h"

using namespace GNet;

/// <summary>
/// Initialise an endpoint on all local addresses.
/// </summary>
/// <param name="port">: predefined to 6112. </param>
GNet::IPEndpoint::IPEndpoint(unsigned short port)
{
	/*
	std::string website_HTML;
	std::locale local;

	char lineBuffer[200][80] = { ' ' };
	char* buffer;
	char ip_address[16] = {0};
	std::string ip;

	int i = 0, bufLen = 0, j = 0, lineCount = 0;
	int lineIndex = 0, posIndex = 0;

	buffer = new char[10000];

	if (GetWebsite("api.ipify.org", website_HTML, buffer))
	{
		for (size_t i = 0; i < website_HTML.length(); ++i) website_HTML[i] = tolower(website_HTML[i], local);

		std::istringstream ss(website_HTML);
		std::string stoken;

		while (getline(ss, stoken, '\n')) {

			strcpy_s(lineBuffer[lineIndex], stoken.c_str());
			int dot = 0;
			for (int ii = 0; ii < strlen(lineBuffer[lineIndex]); ii++) {

				if (lineBuffer[lineIndex][ii] == '.') dot++;
				if (dot >= 3) {
					dot = 0;
					strcpy_s(ip_address, lineBuffer[lineIndex]);
				}
			}
			lineIndex++;
		}

		ip = ip_address;

		delete[] buffer;
	*/

	// Note: Website code not currently required for direct connection and not needed for binding: 
	// You can only bind to addresses local to your machine
	* this = IPEndpoint::IPEndpoint("0.0.0.0", port);
}

bool GNet::IPEndpoint::GetPublicIPFromDomain(std::string url, std::string& website_HTML, char* buffer)
{
	SOCKET Socket;

	std::string get_http;
	
	DWORD dwRetval;
	DWORD dwConnect;

	get_http = "GET / HTTP/1.1\r\nHost: " + url + "\r\nConnection: close\r\n\r\n";

	struct addrinfo* ai;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_NUMERICSERV;

	dwRetval = getaddrinfo(url.c_str(), "80", &hints, &ai);
	
	if (dwRetval != 0) {
		printf("getaddrinfo failed with error: %d\n", dwRetval);
		return false;
	}

	Socket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (Socket < 0)
	{
		printf("Socket failed to create. \n");
		closesocket(Socket);
		return false;
	}

	dwConnect = connect(Socket, ai->ai_addr, ai->ai_addrlen);

	freeaddrinfo(ai);

	if (dwConnect != 0)
	{
		printf("Sockect connection failed with error: %d\n", dwConnect);
		closesocket(Socket);
		return false;
	}

	int bytesSent = send(Socket, get_http.c_str(), strlen(get_http.c_str()), 0);

	if (bytesSent <= 0)
	{
		printf("Could not send data to connected host.\n");
		closesocket(Socket);
		return false;
	}

	int nDataLength;
	while ((nDataLength = recv(Socket, buffer, 10000, 0)) > 0) {
		int i = 0;
		while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {

			website_HTML += buffer[i];
			i += 1;
		}
	}

	closesocket(Socket);
	return true;
}
	

GNet::IPEndpoint::IPEndpoint(const char* ip, unsigned short port)
{
	this->port = port;

#pragma region IPV4
	//IPv4

	in_addr addr; //location to store ipv4 address
	int result = inet_pton(AF_INET, ip, &addr);

	if (result == 1) //ip was valid and was parsed successfully
	{
		if (addr.S_un.S_addr != INADDR_NONE)
		{
			ip_string = ip;
			hostname = ip;

			Helpers::trim(ip_string);
			Helpers::trim(hostname);

			ip_bytes.resize(sizeof(ULONG));
			memcpy(&ip_bytes[0], &addr.S_un.S_addr, sizeof(ULONG));
			ipversion = IPVersion::IPv4;
			return;
		}
	}

	//Resolve hostname
	addrinfo hints = {};
	hints.ai_family = PF_INET; //ipv4 addresses only

	addrinfo* info = nullptr;

	result = getaddrinfo(ip, NULL, &hints, &info);

	if (result == 0)
	{
		sockaddr_in* host_addr = reinterpret_cast<sockaddr_in*>(info->ai_addr);

		ip_string.resize(sizeof(ULONG));
		inet_ntop(PF_INET, &host_addr->sin_addr, &ip_string[0], 16);

		hostname = ip;

		Helpers::trim(ip_string);
		Helpers::trim(hostname);

		ULONG ip_long = host_addr->sin_addr.S_un.S_addr;
		ip_bytes.resize(16);

		memcpy(&ip_bytes[0], &ip_long, sizeof(ULONG));

		ipversion = IPVersion::IPv4;

		freeaddrinfo(info);
		return;
	}
#pragma endregion

#pragma region IPV6
	
	//IPv6

	in6_addr addr6; //location to store ipv6 address
	result = inet_pton(PF_INET6, ip, &addr6);

	if (result == 1) //ip was valid and was parsed successfully
	{
		ip_string = ip;
		hostname = ip;

		Helpers::trim(ip_string);
		Helpers::trim(hostname);

		ip_bytes.resize(16);
		memcpy(&ip_bytes[0], &addr6.u, 16);

		ipversion = IPVersion::IPv6;
		return;
	}

	//Resolve hostname to IPv6 address
	addrinfo hintsv6 = {};
	hints.ai_family = PF_INET6; //ipv6 addresses only

	addrinfo* infov6 = nullptr;

	result = getaddrinfo(ip, NULL, &hintsv6, &infov6);

	if (result == 0)
	{
		sockaddr_in6* host_addr = reinterpret_cast<sockaddr_in6*>(infov6->ai_addr);

		ip_string.resize(46);
		inet_ntop(PF_INET6, &host_addr->sin6_addr, &ip_string[0], 46);

		hostname = ip;

		Helpers::trim(ip_string);
		Helpers::trim(hostname);

		ip_bytes.resize(16);

		memcpy(&ip_bytes[0], &host_addr->sin6_addr, 16);

		ipversion = IPVersion::IPv6;

		freeaddrinfo(info);
		return;
	}
#pragma endregion

}

GNet::IPEndpoint::IPEndpoint(sockaddr* addr)
{
	assert(addr->sa_family == AF_INET || addr->sa_family == AF_INET6);

	if (addr->sa_family == AF_INET) //IPv4
	{
		sockaddr_in* addrv4 = reinterpret_cast<sockaddr_in*>(addr);

		ipversion = IPVersion::IPv4;

		port = ntohs(addrv4->sin_port);

		ip_bytes.resize(sizeof(ULONG));
		memcpy(&ip_bytes[0], &addrv4->sin_addr, sizeof(ULONG));

		ip_string.resize(16);
		inet_ntop(AF_INET, &addrv4->sin_addr, &ip_string[0], 16);

		hostname = ip_string;
	}
	else
	{
		sockaddr_in6* addrv6 = reinterpret_cast<sockaddr_in6*>(addr);

		ipversion = IPVersion::IPv6;

		port = ntohs(addrv6->sin6_port);

		ip_bytes.resize(16);
		memcpy(&ip_bytes[0], &addrv6->sin6_addr, 16);

		ip_string.resize(46);
		inet_ntop(AF_INET6, &addrv6->sin6_addr, &ip_string[0], 46);

		hostname = ip_string;
	}

	Helpers::trim(ip_string);
	Helpers::trim(hostname);
}

IPVersion GNet::IPEndpoint::GetIPVersion()
{
	return ipversion;
}

std::string GNet::IPEndpoint::GetHostname()
{
	return hostname;
}

std::string GNet::IPEndpoint::GetIPString()
{
	return ip_string;
}

std::vector<uint8_t> GNet::IPEndpoint::GetIPBytes()
{
	return ip_bytes;
}

unsigned short GNet::IPEndpoint::GetPort()
{
	return port;
}

sockaddr_in GNet::IPEndpoint::GetSockaddrIPv4()
{
	assert(ipversion == IPVersion::IPv4);

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;

	memcpy(&addr.sin_addr, &ip_bytes[0], sizeof(ULONG)); //copy ip bytes over
	addr.sin_port = htons(port); //convert to network byte order

	return addr;
}

sockaddr_in6 GNet::IPEndpoint::GetSockaddrIPv6()
{
	assert(ipversion == IPVersion::IPv6);

	sockaddr_in6 addr6 = {};
	addr6.sin6_family = AF_INET6;

	memcpy(&addr6.sin6_addr, &ip_bytes[0], 16); //copy ip bytes over
	addr6.sin6_port = htons(port); //convert to network byte order

	return addr6;
}

void GNet::IPEndpoint::Print()
{
	switch (ipversion)
	{
	case IPVersion::IPv4:
		std::cout << "IP Version: IPv4" << std::endl;
		break;
	
	case IPVersion::IPv6:
		std::cout << "IP Version: IPv6" << std::endl;
		break;
	default:
		std::cout << "IP Version: Unknown" << std::endl;
	}
	std::cout << "Hostname: " << hostname << std::endl;
	std::cout << "IP: " << ip_string << std::endl;
	std::cout << "IP bytes...: " << hostname << std::endl;
	std::cout << "Port: " << port << std::endl;

	for (const auto& digit : ip_bytes)
	{
		std::cout << (int)digit << std::endl;
	}
}
