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

    /// A shared pointer to a Connection.
    using ConnectionPtr = std::shared_ptr<Connection>;

    /// A shared pointer to a ConnectionInfo.
    using ConnectionInfoPtr = std::shared_ptr<ConnectionInfo>;

    /// A shared pointer to an IPConnectionInfo.
    using IPConnectionInfoPtr = std::shared_ptr<IPConnectionInfo>;

    /// A shared pointer to a TCPConnectionInfo.
    using TCPConnectionInfoPtr = std::shared_ptr<TCPConnectionInfo>;

    /// A shared pointer to a UDPConnectionInfo.
    using UDPConnectionInfoPtr = std::shared_ptr<UDPConnectionInfo>;

    /// A shared pointer to a WSConnectionInfo.
    using WSConnectionInfoPtr = std::shared_ptr<WSConnectionInfo>;
}

#endif
