#pragma once
#include <stdint.h>

namespace GNet
{
	enum class PacketType : uint16_t //2 bytes per classification
	{
		PT_INVALID,
		PT_VERIFY,
		PT_SCENE_LOAD,
		PT_ENTITY_INSTANTIATE,
		PT_ENTITY_CHANGE,
	};

	inline std::ostream& operator << (std::ostream& os, const PacketType& obj)
	{
		os << static_cast<std::underlying_type<PacketType>::type>(obj);
		return os;
	}
}
