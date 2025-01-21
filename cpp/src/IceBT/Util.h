// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_UTIL_H
#define ICE_BT_UTIL_H

#include "../Ice/Network.h"
#include "Config.h"
#include "IceBT/Types.h"

namespace IceBT
{
    bool isValidDeviceAddress(const std::string&);
    bool parseDeviceAddress(const std::string&, DeviceAddress&);
    std::string formatDeviceAddress(const DeviceAddress&);

    std::string addrToString(const std::string&, std::int32_t);
    std::string addrToString(const SocketAddress&);
    SocketAddress createAddr(const std::string&, std::int32_t);

    std::string fdToString(SOCKET);
    void fdToAddressAndChannel(SOCKET, std::string&, int&, std::string&, int&);
    int compareAddress(const SocketAddress&, const SocketAddress&);
}

#endif
