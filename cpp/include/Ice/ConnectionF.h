//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTION_F_H
#define ICE_CONNECTION_F_H

#include <memory>

namespace Ice
{

    class ConnectionInfo;
    class WSConnectionInfo;
    class Connection;

    using ConnectionInfoPtr = ::std::shared_ptr<ConnectionInfo>;
    using WSConnectionInfoPtr = ::std::shared_ptr<WSConnectionInfo>;
    using ConnectionPtr = ::std::shared_ptr<Connection>;

}

#endif
