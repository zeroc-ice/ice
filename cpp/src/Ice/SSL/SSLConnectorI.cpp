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

    using IceInternal::NetworkRef;

    auto endpoint = NetworkRef<nw_endpoint_t>::adopt(
        nw_endpoint_create_host(nfConnector->host().c_str(), to_string(nfConnector->port()).c_str()));
    if (!endpoint)
    {
        throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
    }

    // The immutable configuration is owned by the blocks Network.framework invokes during the handshake; the
    // results are per connection: the configuration error, and the flag the verification of this connection sets.
    auto configuration =
        make_shared<const IceInternal::NetworkFrameworkTLS::ClientConfiguration>(*clientAuthenticationOptions, _host);
    auto localVerifyRejected = make_shared<atomic<bool>>(false);
    __block exception_ptr error;

    auto parameters = NetworkRef<nw_parameters_t>::adopt(nw_parameters_create_secure_tcp(
        ^(nw_protocol_options_t tlsOptions) {
          error = IceInternal::NetworkFrameworkTLS::configureClientTLS(tlsOptions, configuration, localVerifyRejected);
        },
        NW_PARAMETERS_DEFAULT_CONFIGURATION));

    if (error)
    {
        // Set by the configure block, which nw_parameters_create_secure_tcp invoked synchronously.
        rethrow_exception(error);
    }
    if (!parameters)
    {
        throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
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
            auto proxyEndpoint = NetworkRef<nw_endpoint_t>::adopt(
                nw_endpoint_create_host(proxyHost.c_str(), to_string(proxyPort).c_str()));
            auto proxyConfig = NetworkRef<nw_proxy_config_t>::adopt(
                proxy->getName() == "HTTP" ? nw_proxy_config_create_http_connect(proxyEndpoint.get(), nullptr)
                                           : nw_proxy_config_create_socksv5(proxyEndpoint.get()));
            // The proxy is explicitly configured: never fall back to a direct connection when it cannot be used.
            nw_proxy_config_set_failover_allowed(proxyConfig.get(), false);
            auto privacyContext =
                NetworkRef<nw_privacy_context_t>::adopt(nw_privacy_context_create("com.zeroc.ice.ssl-proxy"));
            nw_privacy_context_add_proxy(privacyContext.get(), proxyConfig.get());
            nw_parameters_set_privacy_context(parameters.get(), privacyContext.get());
        }
        else
        {
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
        auto localEndpoint = NetworkRef<nw_endpoint_t>::adopt(nw_endpoint_create_address(&sourceAddr.sa));
        if (!localEndpoint)
        {
            throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
        }
        nw_parameters_set_local_endpoint(parameters.get(), localEndpoint.get());
    }

    // The transceiver owns the connection.
    auto connection = NetworkRef<nw_connection_t>::adopt(nw_connection_create(endpoint.get(), parameters.get()));
    if (!connection)
    {
        throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
    }

    IceInternal::ProtocolInstancePtr protocolInstance = _instance;
    auto transceiver = make_shared<IceInternal::NetworkFrameworkTransceiver>(
        protocolInstance,
        std::move(connection),
        true /* secure */,
        nfConnector->proxy(),
        nfConnector->address());
    transceiver->setLocalVerifyRejected(std::move(localVerifyRejected));
    SSLEnginePtr engine = _instance->engine();
    transceiver->setPeerVerifier([engine](const ConnectionInfoPtr& info) { engine->verifyPeer(info); }, false, "");
    return transceiver;
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
