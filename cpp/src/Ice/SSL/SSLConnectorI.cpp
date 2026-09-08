// Copyright (c) ZeroC, Inc.

#include "SSLConnectorI.h"
#include "../TargetCompare.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "SSLEngine.h"
#include "SSLInstance.h"

#if defined(ICE_USE_SCHANNEL)
#    include "SchannelEngine.h"
#    include "SchannelTransceiverI.h"
using namespace Ice::SSL::Schannel;
#elif defined(ICE_USE_APPLE_SSL)
#    include "../NetworkProxy.h"
#    include "../apple/NetworkFrameworkConnector.h"
#    include "../apple/NetworkFrameworkTLS.h"
#    include "../apple/NetworkFrameworkTransceiver.h"
#    include <Network/Network.h>
#elif defined(ICE_USE_OPENSSL)
#    include "OpenSSLEngine.h"
#    include "OpenSSLTransceiverI.h"
using namespace Ice::SSL::OpenSSL;
#endif

using namespace std;
using namespace Ice;
using namespace Ice::SSL;

IceInternal::TransceiverPtr
Ice::SSL::ConnectorI::connect()
{
    optional<Ice::SSL::ClientAuthenticationOptions> clientAuthenticationOptions =
        _instance->engine()->getInitializationData().clientAuthenticationOptions;
    if (!clientAuthenticationOptions)
    {
        clientAuthenticationOptions = _instance->engine()->createClientAuthenticationOptions(_host);
    }
    assert(clientAuthenticationOptions);

#if defined(ICE_USE_NETWORK_FRAMEWORK)
    //
    // For Network.framework, TLS is configured in the connection parameters rather than
    // as a transceiver wrapper. Create a TLS-enabled nw_connection_t directly.
    //
    auto* nfConnector = dynamic_cast<const IceInternal::NetworkFrameworkConnector*>(_delegate.get());
    assert(nfConnector);

    nw_endpoint_t endpoint = nw_endpoint_create_host(nfConnector->host().c_str(), to_string(nfConnector->port()).c_str());
    if (!endpoint)
    {
        throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
    }

    // The configuration is shared with the blocks Network.framework invokes during the handshake.
    auto configuration =
        make_shared<IceInternal::NetworkFrameworkTLS::ClientConfiguration>(*clientAuthenticationOptions, _host);

    nw_parameters_t parameters = nw_parameters_create_secure_tcp(
        ^(nw_protocol_options_t tlsOptions) {
          IceInternal::NetworkFrameworkTLS::configureClientTLS(tlsOptions, configuration);
        },
        NW_PARAMETERS_DEFAULT_CONFIGURATION);

    if (!parameters)
    {
        nw_release(endpoint);
        throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
    }

    if (configuration->error)
    {
        // Set by the configure block, which nw_parameters_create_secure_tcp invoked synchronously.
        nw_release(parameters);
        nw_release(endpoint);
        rethrow_exception(configuration->error);
    }

    // Ice performs the SOCKS or HTTP CONNECT proxy handshake in-band on plain TCP connections, before it starts
    // using the connection. This is not possible with a Network.framework TLS connection, whose handshake starts as
    // soon as the transport connection is established. Use Network.framework's own proxy support instead; its SOCKS
    // client also interoperates with the SOCKS4 proxy of the Ice test suite.
    if (const IceInternal::NetworkProxyPtr& proxy = nfConnector->proxy())
    {
        if (__builtin_available(macOS 14.0, iOS 17.0, tvOS 17.0, watchOS 10.0, *))
        {
            string proxyHost;
            int proxyPort;
            IceInternal::addrToAddressAndPort(proxy->getAddress(), proxyHost, proxyPort);
            nw_endpoint_t proxyEndpoint = nw_endpoint_create_host(proxyHost.c_str(), to_string(proxyPort).c_str());
            nw_proxy_config_t proxyConfig = proxy->getName() == "HTTP"
                                                ? nw_proxy_config_create_http_connect(proxyEndpoint, nullptr)
                                                : nw_proxy_config_create_socksv5(proxyEndpoint);
            nw_release(proxyEndpoint);
            // The proxy is explicitly configured: never fall back to a direct connection when it cannot be used.
            nw_proxy_config_set_failover_allowed(proxyConfig, false);
            nw_privacy_context_t privacyContext = nw_privacy_context_create("com.zeroc.ice.ssl-proxy");
            nw_privacy_context_add_proxy(privacyContext, proxyConfig);
            nw_parameters_set_privacy_context(parameters, privacyContext);
            nw_release(privacyContext);
            nw_release(proxyConfig);
        }
        else
        {
            nw_release(parameters);
            nw_release(endpoint);
            throw Ice::FeatureNotSupportedException(
                __FILE__,
                __LINE__,
                "SSL connections through a network proxy require macOS 14 or iOS 17");
        }
    }

    // Bind the connection to the source address configured on the endpoint (--sourceAddress), if any.
    const IceInternal::Address& sourceAddr = nfConnector->sourceAddress();
    if (IceInternal::isAddressValid(sourceAddr))
    {
        nw_endpoint_t localEndpoint = nw_endpoint_create_address(&sourceAddr.sa);
        if (!localEndpoint)
        {
            nw_release(parameters);
            nw_release(endpoint);
            throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
        }
        nw_parameters_set_local_endpoint(parameters, localEndpoint);
        nw_release(localEndpoint);
    }

    nw_connection_t connection = nw_connection_create(endpoint, parameters);
    nw_release(endpoint);
    nw_release(parameters);

    if (!connection)
    {
        throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
    }

    try
    {
        IceInternal::ProtocolInstancePtr protocolInstance = _instance;
        auto transceiver = make_shared<IceInternal::NetworkFrameworkTransceiver>(
            protocolInstance,
            connection,
            true /* secure */,
            nfConnector->proxy(),
            nfConnector->address());
        transceiver->setLocalVerifyRejected(configuration->verification.localVerifyRejected);
        SSLEnginePtr engine = _instance->engine();
        transceiver->setPeerVerifier(
            [engine](const ConnectionInfoPtr& info) { engine->verifyPeer(info); },
            false,
            "");
        nw_release(connection);
        return transceiver;
    }
    catch (...)
    {
        nw_release(connection);
        throw;
    }
#else
    return make_shared<TransceiverI>(_instance, _delegate->connect(), _host, *clientAuthenticationOptions);
#endif
}

int16_t
Ice::SSL::ConnectorI::type() const
{
    return _delegate->type();
}

string
Ice::SSL::ConnectorI::toString() const
{
    return _delegate->toString();
}

bool
Ice::SSL::ConnectorI::operator==(const IceInternal::Connector& r) const
{
    const auto* p = dynamic_cast<const ConnectorI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    return Ice::targetEqualTo(_delegate, p->_delegate);
}

bool
Ice::SSL::ConnectorI::operator<(const IceInternal::Connector& r) const
{
    const auto* p = dynamic_cast<const ConnectorI*>(&r);
    if (!p)
    {
        return type() < r.type();
    }

    if (this == p)
    {
        return false;
    }

    return Ice::targetLess(_delegate, p->_delegate);
}

Ice::SSL::ConnectorI::ConnectorI(InstancePtr instance, IceInternal::ConnectorPtr del, string h)
    : _instance(std::move(instance)),
      _delegate(std::move(del)),
      _host(std::move(h))
{
}

Ice::SSL::ConnectorI::~ConnectorI() = default;
