#pragma once
#define WIN32_LEAN_AND_MEAN

#include <vector>
#include <WinSock2.h>
#include <string>
#include "PacketException.h"
#include "PacketType.h"

namespace GNet
{
	class Packet
	{
	public:

		Packet(PacketType packetType = PacketType::PT_INVALID);
		PacketType GetPacketType();
		void SetPacketType(PacketType packetType);

		void Clear();
		void Append(const void* data, uint32_t size);	

		Packet& operator << (uint32_t data); //insertion
		Packet& operator >> (uint32_t& data); //extraction

		Packet& operator << (const std::string &data); //insertion
		Packet& operator >> (std::string &data); //extraction

		//std::vector<char>* GetBuffer() { return &buffer; }

		std::vector<char> buffer;

	private:

		uint32_t extractionOffset = 0; //used to "jump over" data that has already been extracted
	};
}
