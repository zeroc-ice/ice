// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_TLS_H
#define ICE_NETWORK_FRAMEWORK_TLS_H

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

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
namespace IceInternal::NetworkFrameworkTLS
{
    // The verification of the certificate chain the peer presents, performed by the TLS verify block.
    struct PeerVerification
    {
        const bool incoming;
        // The target host for an outgoing connection (the host name policy applies when set), the adapter name for
        // an incoming connection.
        const std::string name;
        // Anchors the trust evaluation when set; retained by the configuration that owns this verification.
        CFArrayRef trustedRootCertificates;
        // The application's validation callback; SecTrustEvaluateWithError when not set.
        const std::function<bool(SecTrustRef, const Ice::SSL::ConnectionInfoPtr&)> callback;
        // Set when this verification rejects the peer, so that the connection reports a SecurityException rather
        // than a ConnectionLostException. Optional.
        const std::shared_ptr<std::atomic<bool>> localVerifyRejected;
    };

    // The TLS configuration of an outgoing connection.
    struct ClientConfiguration
    {
        ClientConfiguration(Ice::SSL::ClientAuthenticationOptions options, std::string host);
        ~ClientConfiguration();
        ClientConfiguration(const ClientConfiguration&) = delete;
        ClientConfiguration& operator=(const ClientConfiguration&) = delete;

        const Ice::SSL::ClientAuthenticationOptions options;
        const std::string host;
        const PeerVerification verification;

        // Set by configureClientTLS when the configuration failed, for example when the certificate selection
        // callback returned an invalid chain or threw. nw_parameters_create_secure_tcp invokes its configure block
        // synchronously: the connector rethrows this exception once the parameters are created, an exception must
        // not escape through Network.framework.
        std::exception_ptr error;
    };

    // The TLS configuration of a listener, shared by the connections it accepts.
    struct ServerConfiguration
    {
        ServerConfiguration(Ice::SSL::ServerAuthenticationOptions options, std::string adapterName, Ice::LoggerPtr);
        ~ServerConfiguration();
        ServerConfiguration(const ServerConfiguration&) = delete;
        ServerConfiguration& operator=(const ServerConfiguration&) = delete;

        const Ice::SSL::ServerAuthenticationOptions options;
        const std::string adapterName;
        const Ice::LoggerPtr logger;
        const PeerVerification verification;

        // See ClientConfiguration::error; the acceptor rethrows it once the listener parameters are created.
        std::exception_ptr error;
    };

    // Configures the TLS options of an outgoing connection: the server name, the client identity, the server
    // certificate verification, then the application's sslNewSessionCallback. Called from the configure block of
    // nw_parameters_create_secure_tcp.
    void configureClientTLS(nw_protocol_options_t tlsOptions, const std::shared_ptr<ClientConfiguration>&);

    // Configures the TLS options of a listener: the server identity, selected for each handshake by a challenge
    // block so that a reloaded certificate is picked up without recreating the listener, the client certificate
    // requirement and verification, then the application's sslNewSessionCallback. Called from the configure block
    // of nw_parameters_create_secure_tcp.
    void configureServerTLS(nw_protocol_options_t tlsOptions, const std::shared_ptr<ServerConfiguration>&);
}

#endif

#endif
