// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_CONFIG_H
#define ICE_BT_CONFIG_H

#include <Ice/Config.h>
#include <Ice/Network.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

namespace IceBT
{

typedef bdaddr_t DeviceAddress;

typedef sockaddr_rc SocketAddress;

}

#endif
