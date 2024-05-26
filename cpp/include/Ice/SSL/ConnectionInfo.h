//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param peerCertificate The peer certificate.
         */
        SchannelConnectionInfo(
            const Ice::ConnectionInfoPtr& underlying,
            bool incoming,
            const std::string& adapterName,
            const std::string& connectionId,
            PCCERT_CONTEXT peerCertificate)
            : Ice::ConnectionInfo(underlying, incoming, adapterName, connectionId),
              peerCertificate(peerCertificate)
        {
        }

        ~SchannelConnectionInfo() override;

        SchannelConnectionInfo(const SchannelConnectionInfo&) = delete;
        SchannelConnectionInfo& operator=(const SchannelConnectionInfo&) = delete;

        /**
         * The peer certificate.
         */
        PCCERT_CONTEXT peerCertificate = nullptr;
    };
#elif defined(ICE_USE_SECURE_TRANSPORT)
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

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param peerCertificate The peer certificate.
         */
        SecureTransportConnectionInfo(
            const Ice::ConnectionInfoPtr& underlying,
            bool incoming,
            const std::string& adapterName,
            const std::string& connectionId,
            SecCertificateRef peerCertificate)
            : Ice::ConnectionInfo(underlying, incoming, adapterName, connectionId),
              peerCertificate(peerCertificate)
        {
        }

        ~SecureTransportConnectionInfo() override;

        SecureTransportConnectionInfo(const SecureTransportConnectionInfo&) = delete;
        SecureTransportConnectionInfo& operator=(const SecureTransportConnectionInfo&) = delete;

        /**
         * The peer certificate.
         */
        SecCertificateRef peerCertificate = nullptr;
    };
#elif defined(ICE_USE_OPENSSL)
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

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param peerCertificate The peer certificate.
         */
        OpenSSLConnectionInfo(
            const Ice::ConnectionInfoPtr& underlying,
            bool incoming,
            const std::string& adapterName,
            const std::string& connectionId,
            X509* peerCertificate)
            : Ice::ConnectionInfo(underlying, incoming, adapterName, connectionId),
              peerCertificate(peerCertificate)
        {
        }

        ~OpenSSLConnectionInfo() override;

        OpenSSLConnectionInfo(const OpenSSLConnectionInfo&) = delete;
        OpenSSLConnectionInfo& operator=(const OpenSSLConnectionInfo&) = delete;

        /**
         * The peer certificate.
         */
        X509* peerCertificate = nullptr;
    };
#else
#    error "unsupported platform"
#endif
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
