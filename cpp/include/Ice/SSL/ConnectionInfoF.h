//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_CONNECTION_INFO_F_H
#define ICE_SSL_CONNECTION_INFO_F_H

#include "Config.h"

#include <memory>

namespace Ice::SSL
{
#if defined(ICE_USE_SCHANNEL)
    class SchannelConnectionInfo;
    // Alias for portable code
    using ConnectionInfo = SchannelConnectionInfo;
    using ConnectionInfoPtr = std::shared_ptr<SchannelConnectionInfo>;
#elif defined(ICE_USE_SECURE_TRANSPORT)
    class SecureTransportConnectionInfo;
    // Alias for portable code
    using ConnectionInfo = SecureTransportConnectionInfo;
    using ConnectionInfoPtr = std::shared_ptr<SecureTransportConnectionInfo>;
#else
    class OpenSSLConnectionInfo;
    // Alias for portable code
    using ConnectionInfo = OpenSSLConnectionInfo;
    using ConnectionInfoPtr = std::shared_ptr<OpenSSLConnectionInfo>;
#endif
}

#endif
