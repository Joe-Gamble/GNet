#pragma once
#define WIN32_LEAN_AND_MEAN

#include <vector>
#include <WinSock2.h>
#include <string>

namespace GNet
{
	class Packet
	{
	public:
		void Clear();
		void Append(const void* data, uint32_t size);	

		Packet& operator << (uint32_t data); //insertion
		Packet& operator >> (uint32_t& data); //extraction

		Packet& operator << (const std::string &data); //insertion
		Packet& operator >> (std::string &data); //extraction

		uint32_t extractionOffset = 0; //used to "jump over" data that has already been extracted
		std::vector<char> buffer;
	};
}
