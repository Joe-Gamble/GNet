#include "PacketManager.h"

namespace GNet
{
	void PacketManager::Clear()
	{
		packets = std::queue<std::shared_ptr<Packet>>{}; //Clear the queue of packets
	}

	void PacketManager::Append(std::shared_ptr<Packet> p)
	{
		packets.push(std::move(p)); //Add packet to queue.
	}

	std::shared_ptr<Packet> PacketManager::Retrieve()
	{
		std::shared_ptr<Packet> packet = packets.front(); //Get packet from front of the queue
		return packet; // Return packet that was removed from the queue
	}

	bool PacketManager::HasPendingPackets()
	{
		return (!packets.empty());
	}

	void PacketManager::Pop()
	{
		packets.pop(); //Remove packet from front of the queue.
	}
}