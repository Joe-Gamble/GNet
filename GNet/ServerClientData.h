#pragma once
#include "IPEndpoint.h"

namespace GNet
{
    struct ServerClientData
    {
        IPEndpoint address;
        uint64_t clientSalt;
        uint64_t challengeSalt;
        double connectTime;
        double lastPacketSendTime;
        double lastPacketReceiveTime;

        ServerClientData()
        {
            clientSalt = 0;
            challengeSalt = 0;
            connectTime = 0.0;
            lastPacketSendTime = 0.0;
            lastPacketReceiveTime = 0.0;
        }
    };
}
