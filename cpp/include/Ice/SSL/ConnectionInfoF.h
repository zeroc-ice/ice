// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_CONNECTION_INFO_F_H
#define ICE_SSL_CONNECTION_INFO_F_H

#include "Config.h"

#include <memory>

namespace Ice::SSL
{
#if defined(ICE_USE_SCHANNEL)
    class SchannelConnectionInfo;
    /// \cond INTERNAL
    // Alias for the platform-specific implementation of ConnectionInfo on Windows.
    using ConnectionInfo = SchannelConnectionInfo;
    using ConnectionInfoPtr = std::shared_ptr<SchannelConnectionInfo>;
    /// \endcond
#elif defined(ICE_USE_SECURE_TRANSPORT)
    class SecureTransportConnectionInfo;
    /// \cond INTERNAL
    // Alias for the platform-specific implementation of ConnectionInfo on macOS and iOS.
    using ConnectionInfo = SecureTransportConnectionInfo;
    using ConnectionInfoPtr = std::shared_ptr<SecureTransportConnectionInfo>;
    /// \endcond
#else
    class OpenSSLConnectionInfo;
    /// \cond INTERNAL
    // Alias for the platform-specific implementation of ConnectionInfo on Linux.
    using ConnectionInfo = OpenSSLConnectionInfo;
    using ConnectionInfoPtr = std::shared_ptr<OpenSSLConnectionInfo>;
    /// \endcond
#endif
}

#endif
