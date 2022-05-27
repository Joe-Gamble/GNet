#pragma once

#include <queue>
#include <memory>
#include "Packet.h"

namespace GNet
{
	enum PacketManagerTask
	{
		ProcessPacketSize,
		ProcessPacketContents
	};

	class PacketManager
	{
	public:
		void Clear();
		bool HasPendingPackets();
		void Append(std::shared_ptr<Packet> p);
		std::shared_ptr<Packet> Retrieve();
		void Pop();

		uint16_t currentPacketSize = 0;
		int currentPacketExtrtactionOffset = 0;
		PacketManagerTask currentTask = PacketManagerTask::ProcessPacketSize;

	private:
		std::queue<std::shared_ptr<Packet>> packets;
	};
}
class PacketManager
{
};

