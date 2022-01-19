#include "Packet.h"

using namespace GNet;

void Packet::Clear()
{
	buffer.clear();
	extractionOffset = 0;
}

void Packet::Append(const void* data, uint32_t size)
{
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

	data.resize(stringSize);
	data.assign(&buffer[extractionOffset], stringSize); //assign data at offset into data variable
	extractionOffset += stringSize;
	return *this;
}