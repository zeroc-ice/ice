//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_CONNECTION_INFO_H
#define ICESSL_CONNECTION_INFO_H

#include "ConnectionInfoF.h"
#include "Ice/Connection.h"
#include "Plugin.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace IceSSL
{
    /**
     * Provides access to the connection details of an SSL connection
     * \headerfile IceSSL/IceSSL.h
     */
    class ConnectionInfo : public Ice::ConnectionInfo
    {
    public:
        ConnectionInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param cipher The negotiated cipher suite.
         * @param certs The certificate chain.
         * @param verified The certificate chain verification status.
         */
        ConnectionInfo(
            const Ice::ConnectionInfoPtr& underlying,
            bool incoming,
            const std::string& adapterName,
            const std::string& connectionId,
            const std::string& cipher,
            const std::vector<CertificatePtr>& certs,
            bool verified)
            : Ice::ConnectionInfo(underlying, incoming, adapterName, connectionId),
              cipher(cipher),
              certs(certs),
              verified(verified)
        {
        }

        ConnectionInfo(const ConnectionInfo&) = delete;
        ConnectionInfo& operator=(const ConnectionInfo&) = delete;

        /**
         * The negotiated cipher suite.
         */
        std::string cipher;
        /**
         * The certificate chain.
         */
        std::vector<CertificatePtr> certs;
        /**
         * The certificate chain verification status.
         */
        bool verified;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
