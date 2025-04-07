// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_CONNECTION_INFO_H
#define ICE_SSL_CONNECTION_INFO_H

#include "Config.h"
#include "ConnectionInfoF.h"
#include "Ice/Connection.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

// This file defines the `XxxConnectionInfo` class for each platform-specific SSL implementation. The
// `#if defined(ICE_USE_XXX)/#endif` directives are used to include the appropriate structure based on the platform. We
// avoid using `#elif` directives because we want to define all the classes when building the doxygen documentation.

/// Secure connections with SSL/TLS.
namespace Ice::SSL
{
#if defined(ICE_USE_SCHANNEL) || defined(ICE_DOXYGEN)
    /// Provides access to the connection details of an SSL connection.
    ///
    /// The SchannelConnectionInfo class is only available when the Ice library is built on Windows. For Linux,
    /// refer to OpenSSLConnectionInfo, and for macOS and iOS, refer to SecureTransportConnectionInfo.
    /// @see ::Ice::SSL::ConnectionInfo
    class ICE_API SchannelConnectionInfo final : public Ice::ConnectionInfo
    {
    public:
        ~SchannelConnectionInfo() final;
        SchannelConnectionInfo(const SchannelConnectionInfo&) = delete;
        SchannelConnectionInfo& operator=(const SchannelConnectionInfo&) = delete;

        /// The peer certificate.
        const PCCERT_CONTEXT peerCertificate;

        /// @private
        SchannelConnectionInfo(Ice::ConnectionInfoPtr underlying, PCCERT_CONTEXT peerCertificate)
            : ConnectionInfo{std::move(underlying)},
              peerCertificate{peerCertificate}
        {
        }
    };
#endif

#if defined(ICE_USE_SECURE_TRANSPORT) || defined(ICE_DOXYGEN)
    /// Provides access to the connection details of an SSL connection.
    ///
    /// The SecureTransportConnectionInfo class is only available when the Ice library is built on macOS or iOS. For
    /// Linux, refer to OpenSSLConnectionInfo, and for Windows, refer to SchannelConnectionInfo.
    /// @see ::Ice::SSL::ConnectionInfo
    class ICE_API SecureTransportConnectionInfo final : public Ice::ConnectionInfo
    {
    public:
        ~SecureTransportConnectionInfo() final;
        SecureTransportConnectionInfo(const SecureTransportConnectionInfo&) = delete;
        SecureTransportConnectionInfo& operator=(const SecureTransportConnectionInfo&) = delete;

        /// The peer certificate.
        const SecCertificateRef peerCertificate;

        /// @private
        SecureTransportConnectionInfo(Ice::ConnectionInfoPtr underlying, SecCertificateRef peerCertificate)
            : ConnectionInfo{std::move(underlying)},
              peerCertificate{peerCertificate}
        {
        }
    };
#endif

#if defined(ICE_USE_OPENSSL) || defined(ICE_DOXYGEN)
    /// Provides access to the connection details of an SSL connection.
    ///
    /// The OpenSSLConnectionInfo class is only available when the Ice library is built on Linux. For Windows,
    /// refer to SchannelConnectionInfo, and for macOS and iOS, refer to SecureTransportConnectionInfo.
    /// @see ::Ice::SSL::ConnectionInfo
    class ICE_API OpenSSLConnectionInfo final : public Ice::ConnectionInfo
    {
    public:
        ~OpenSSLConnectionInfo() final;
        OpenSSLConnectionInfo(const OpenSSLConnectionInfo&) = delete;
        OpenSSLConnectionInfo& operator=(const OpenSSLConnectionInfo&) = delete;

        /// The peer certificate.
        X509* const peerCertificate;

        /// @private
        OpenSSLConnectionInfo(Ice::ConnectionInfoPtr underlying, X509* peerCertificate)
            : ConnectionInfo{std::move(underlying)},
              peerCertificate{peerCertificate}
        {
        }
    };
#endif
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
