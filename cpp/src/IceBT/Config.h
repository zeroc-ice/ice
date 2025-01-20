// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_CONFIG_H
#define ICE_BT_CONFIG_H

#include "../Ice/Network.h"
#include "Ice/Config.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

namespace IceBT
{
    typedef bdaddr_t DeviceAddress;

    typedef sockaddr_rc SocketAddress;
}

#endif
