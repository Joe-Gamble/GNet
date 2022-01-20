#include "Packet.h"	
#include "Constants.h"

using namespace GNet;

Packet::Packet(PacketType packetType)
{
	Clear();
	SetPacketType(packetType);
}

PacketType Packet::GetPacketType()
{
	PacketType* typePtr = reinterpret_cast<PacketType*>(&buffer[0]);
	return static_cast<PacketType>(ntohs((uint16_t )*typePtr));
}

void Packet::SetPacketType(PacketType packetType)
{
	PacketType* typePtr = reinterpret_cast<PacketType*>(&buffer[0]);
	*typePtr = static_cast<PacketType>(htons((uint16_t)packetType));
}

void Packet::Clear()
{
	buffer.resize(sizeof(PacketType));
	SetPacketType(PacketType::PT_INVALID);
	extractionOffset = sizeof(PacketType);
}

void Packet::Append(const void* data, uint32_t size)
{
	if (buffer.size() + size > GNet::g_MaxPacketSize) //is the data being added going to exceed the max packet size
		throw PacketException("[Packet::Append(const void* data, uint32_t size)] - Packet size exceeded max packet size.");

	buffer.insert(buffer.end(), (char*)data, (char*)data + size);
}

Packet& Packet::operator<<(uint32_t data)
{
	data = htonl(data); //host to network byte order
	Append(&data, sizeof(uint32_t)); //Insert data into packet
	return *this;
}

Packet& Packet::operator>>(uint32_t& data)
{
	if((extractionOffset + sizeof(uint32_t)) > buffer.size())
		throw PacketException("[Packet::operator>>(uint32_t &)] - Extraction offset exceeded buffer size.");

	data = *reinterpret_cast<uint32_t*>(&buffer[extractionOffset]); //cast from char to uint32
	data = ntohl(data); //network to host byte order

	extractionOffset += sizeof(uint32_t);
	return *this;
}

Packet& Packet::operator<<(const std::string& data)
{
	*this << (uint32_t)data.size();
	Append(data.data(), data.size());
	return *this;
}

Packet& Packet::operator>>(std::string& data) //extract into this string
{
	data.clear(); //clear original contents if any 

	uint32_t stringSize = 0;
	*this >> stringSize; //dont need to increment eo here as overload does it for us

	if ((extractionOffset + stringSize) > buffer.size())
		throw PacketException("[Packet::operator>>(std::string &)] - Extraction offset exceeded buffer size.");

	data.resize(stringSize);
	data.assign(&buffer[extractionOffset], stringSize); //assign data at offset into data variable
	extractionOffset += stringSize;
	return *this;
}