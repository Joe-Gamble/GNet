#pragma once
#include <stdint.h>

namespace GNet
{
	enum class PacketType : uint16_t //2 bytes per classification
	{
		PT_INVALID,
		PT_CHATMESSAGE,
		PT_INTEGERARRAY 
	};
}
