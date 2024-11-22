//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_CONNECTION_INFO_H
#define ICE_SSL_CONNECTION_INFO_H

#include "Config.h"
#include "ConnectionInfoF.h"
#include "Ice/Connection.h"

// This file defines the `XxxConnectionInfo` class for each platform-specific SSL implementation. The
// `#if defined(ICE_USE_XXX)/#endif` directives are used to include the appropriate structure based on the platform. We
// avoid using `#elif` directives because, we want to define all the classes when building the doxygen documentation.

namespace Ice::SSL
{
#if defined(ICE_USE_SCHANNEL)
    /**
     * Provides access to the connection details of an SSL connection.
     *
     * The SchannelConnectionInfo class is only available when the %Ice library is built on Windows. For Linux,
     * refer to OpenSSLConnectionInfo, and for macOS and iOS, refer to SecureTransportConnectionInfo.
     *
     * Additionally, the `ConnectionInfo` alias is defined for use in portable code, representing the
     * platform-specific connection info class.
     */
    class ICE_API SchannelConnectionInfo final : public Ice::ConnectionInfo
    {
    public:
        SchannelConnectionInfo() = default;
        ~SchannelConnectionInfo() final;
        SchannelConnectionInfo(const SchannelConnectionInfo&) = delete;
        SchannelConnectionInfo& operator=(const SchannelConnectionInfo&) = delete;

        /**
         * The peer certificate.
         */
        PCCERT_CONTEXT peerCertificate = nullptr;
    };
#endif

#if defined(ICE_USE_SECURE_TRANSPORT)
    /**
     * Provides access to the connection details of an SSL connection.
     *
     * The SecureTransportConnectionInfo class is only available when the %Ice library is built on macOS or iOS. For
     * Linux, refer to OpenSSLConnectionInfo, and for Windows, refer to SchannelConnectionInfo.
     *
     * Additionally, the `ConnectionInfo` alias is defined for use in portable code, representing the
     * platform-specific connection info class.
     */
    class ICE_API SecureTransportConnectionInfo final : public Ice::ConnectionInfo
    {
    public:
        SecureTransportConnectionInfo() = default;
        ~SecureTransportConnectionInfo() final;
        SecureTransportConnectionInfo(const SecureTransportConnectionInfo&) = delete;
        SecureTransportConnectionInfo& operator=(const SecureTransportConnectionInfo&) = delete;

        /**
         * The peer certificate.
         */
        SecCertificateRef peerCertificate = nullptr;
    };
#endif

#if defined(ICE_USE_OPENSSL)
    /**
     * Provides access to the connection details of an SSL connection.
     *
     * The OpenSSLConnectionInfo class is only available when the %Ice library is built on Linux. For Windows,
     * refer to SchannelConnectionInfo, and for macOS and iOS, refer to SecureTransportConnectionInfo.
     *
     * Additionally, the `ConnectionInfo` alias is defined for use in portable code, representing the
     * platform-specific connection info class.
     */
    class ICE_API OpenSSLConnectionInfo final : public Ice::ConnectionInfo
    {
    public:
        OpenSSLConnectionInfo() = default;
        ~OpenSSLConnectionInfo() final;
        OpenSSLConnectionInfo(const OpenSSLConnectionInfo&) = delete;
        OpenSSLConnectionInfo& operator=(const OpenSSLConnectionInfo&) = delete;

        /**
         * The peer certificate.
         */
        X509* peerCertificate = nullptr;
    };
#endif
}

#endif
