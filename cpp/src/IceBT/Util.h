// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_BT_UTIL_H
#define ICE_BT_UTIL_H

#include <IceBT/Config.h>
#include <IceBT/Types.h>

#include <Ice/Network.h>

namespace IceBT
{

bool isValidDeviceAddress(const std::string&);
bool parseDeviceAddress(const std::string&, DeviceAddress&);
std::string formatDeviceAddress(const DeviceAddress&);

std::string addrToString(const std::string&, Ice::Int);
std::string addrToString(const SocketAddress&);
SocketAddress createAddr(const std::string&, Ice::Int);

std::string fdToString(SOCKET);
void fdToAddressAndChannel(SOCKET, std::string&, int&, std::string&, int&);
int compareAddress(const SocketAddress&, const SocketAddress&);

}

#endif
