// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_CONNECTION_INFO_F_H
#define ICE_SSL_CONNECTION_INFO_F_H

#include "Config.h"

#include <memory>

namespace Ice::SSL
{
#if defined(ICE_DOXYGEN)
    /// An alias for the platform-specific implementation of the SSL %ConnectionInfo.
    using ConnectionInfo = ...;

    /// A shared pointer to a #ConnectionInfo.
    using ConnectionInfoPtr = std::shared_ptr<ConnectionInfo>;
#elif defined(ICE_USE_SCHANNEL)
    class SchannelConnectionInfo;

    /// An alias for the platform-specific implementation of the SSL ConnectionInfo on Windows.
    using ConnectionInfo = SchannelConnectionInfo;
    using ConnectionInfoPtr = std::shared_ptr<SchannelConnectionInfo>;
#elif defined(ICE_USE_APPLE_SSL)
    class AppleConnectionInfo;

    /// An alias for the platform-specific implementation of the SSL ConnectionInfo on macOS and iOS.
    using ConnectionInfo = AppleConnectionInfo;
    using ConnectionInfoPtr = std::shared_ptr<AppleConnectionInfo>;
#else
    class OpenSSLConnectionInfo;

    /// An alias for the platform-specific implementation of the SSL ConnectionInfo on Linux.
    using ConnectionInfo = OpenSSLConnectionInfo;
    using ConnectionInfoPtr = std::shared_ptr<OpenSSLConnectionInfo>;
#endif
}

#endif
