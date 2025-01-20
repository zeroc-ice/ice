// Copyright (c) ZeroC, Inc.

#ifndef ICE_CONNECTION_F_H
#define ICE_CONNECTION_F_H

#include <memory>

namespace Ice
{
    class Connection;
    class ConnectionInfo;
    class IPConnectionInfo;
    class TCPConnectionInfo;
    class UDPConnectionInfo;
    class WSConnectionInfo;

    using ConnectionPtr = std::shared_ptr<Connection>;
    using ConnectionInfoPtr = std::shared_ptr<ConnectionInfo>;
    using IPConnectionInfoPtr = std::shared_ptr<IPConnectionInfo>;
    using TCPConnectionInfoPtr = std::shared_ptr<TCPConnectionInfo>;
    using UDPConnectionInfoPtr = std::shared_ptr<UDPConnectionInfo>;
    using WSConnectionInfoPtr = std::shared_ptr<WSConnectionInfo>;
}

#endif
