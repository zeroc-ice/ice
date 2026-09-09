// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_TLS_H
#define ICE_NETWORK_FRAMEWORK_TLS_H

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "../UniqueRef.h"
#    include "Ice/Logger.h"
#    include "Ice/SSL/ClientAuthenticationOptions.h"
#    include "Ice/SSL/ConnectionInfoF.h"
#    include "Ice/SSL/ServerAuthenticationOptions.h"

#    include <Network/Network.h>

#    include <atomic>
#    include <exception>
#    include <functional>
#    include <memory>
#    include <string>

// The TLS configuration of the Network.framework connections and listeners. The client and server entry points
// stay separate, their policies differ: the host name policy, the client certificate requirement, the listener
// configuration shared by the accepted connections, and when the local identity is selected. They share the peer
// verification, which owns these invariants: everything a block invoked asynchronously by Network.framework uses is
// retained through the configuration, the verification completion is invoked exactly once, an application exception
// fails the handshake, and a rejection by the local verification is distinguished from the other failures.
//
// The configurations are immutable and owned by the blocks that capture them: a client configuration lives as long
// as the handshake of its connection, a server configuration as long as the listener and the handshakes of the
// connections it accepted. The results of a configuration or of a handshake are per connection and live in separate
// state: the exception the configure functions return, and the flag recording a rejection by the local verification.
namespace IceInternal::NetworkFrameworkTLS
{
    // The verification of the certificate chain the peer presents, performed by the TLS verify block.
    struct PeerVerification
    {
        const bool incoming;
        // The target host for an outgoing connection (the host name policy applies when set), the adapter name for
        // an incoming connection.
        const std::string name;
        // Anchors the trust evaluation when set. Retained: the application may release its own reference.
        const UniqueRef<CFArrayRef> trustedRootCertificates;
        // The application's validation callback; SecTrustEvaluateWithError when not set.
        const std::function<bool(SecTrustRef, const Ice::SSL::ConnectionInfoPtr&)> callback;
    };

    // The TLS configuration of an outgoing connection.
    struct ClientConfiguration
    {
        ClientConfiguration(Ice::SSL::ClientAuthenticationOptions options, std::string host);

        const Ice::SSL::ClientAuthenticationOptions options;
        const std::string host;
        const PeerVerification verification;
    };

    // The TLS configuration of a listener, shared by the connections it accepts.
    struct ServerConfiguration
    {
        ServerConfiguration(Ice::SSL::ServerAuthenticationOptions options, std::string adapterName, Ice::LoggerPtr);

        const Ice::SSL::ServerAuthenticationOptions options;
        const std::string adapterName;
        const Ice::LoggerPtr logger;
        const PeerVerification verification;
    };

    // Configures the TLS options of an outgoing connection: the server name, the client identity, the server
    // certificate verification, then the application's sslNewSessionCallback. Called from the configure block of
    // nw_parameters_create_secure_tcp, which invokes it synchronously. localVerifyRejected is set when the
    // verification of this connection rejects the server, so that the connection reports a SecurityException rather
    // than a ConnectionLostException.
    //
    // Returns the exception the configuration failed with, for example when the certificate selection callback
    // returned an invalid chain or threw: an exception must not escape through Network.framework, the connector
    // throws it once the parameters are created.
    std::exception_ptr configureClientTLS(
        nw_protocol_options_t tlsOptions,
        std::shared_ptr<const ClientConfiguration>,
        std::shared_ptr<std::atomic<bool>> localVerifyRejected);

    // Configures the TLS options of a listener: the server identity, selected for each handshake by a challenge
    // block so that a reloaded certificate is picked up without recreating the listener, the client certificate
    // requirement and verification, then the application's sslNewSessionCallback. Called from the configure block
    // of nw_parameters_create_secure_tcp, which invokes it synchronously. Returns the exception the configuration
    // failed with, see configureClientTLS.
    std::exception_ptr configureServerTLS(nw_protocol_options_t tlsOptions, std::shared_ptr<const ServerConfiguration>);
}

#endif

#endif
