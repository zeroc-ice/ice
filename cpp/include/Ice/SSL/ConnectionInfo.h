//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_CONNECTION_INFO_H
#define ICE_SSL_CONNECTION_INFO_H

#include "Certificate.h"
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
    /**
     * Provides access to the connection details of an SSL connection.
     */
    class ICE_API ConnectionInfo : public Ice::ConnectionInfo
    {
    public:
        ConnectionInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param certs The certificate chain.
         */
        ConnectionInfo(
            const Ice::ConnectionInfoPtr& underlying,
            bool incoming,
            const std::string& adapterName,
            const std::string& connectionId,
            const std::vector<CertificatePtr>& certs)
            : Ice::ConnectionInfo(underlying, incoming, adapterName, connectionId),
              certs(certs)
        {
        }

        ~ConnectionInfo() override;

        ConnectionInfo(const ConnectionInfo&) = delete;
        ConnectionInfo& operator=(const ConnectionInfo&) = delete;

        /**
         * The certificate chain.
         */
        std::vector<CertificatePtr> certs;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
