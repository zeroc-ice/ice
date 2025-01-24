// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_CONFIG_H
#define ICE_BT_CONFIG_H

#include "../Ice/Network.h"
#include "Ice/Config.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

namespace IceBT
{
    using DeviceAddress = bdaddr_t;

    using SocketAddress = sockaddr_rc;
}

#endif
