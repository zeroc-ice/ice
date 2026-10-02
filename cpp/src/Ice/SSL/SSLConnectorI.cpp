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
    // With Network.framework, TLS is a protocol of the connection parameters rather than a transceiver wrapping a
    // plain connection: the connector configures the TLS parameters and the shared connector establishes the
    // connection with them.
    auto* nfConnector = dynamic_cast<const IceInternal::NetworkFrameworkConnector*>(_delegate.get());
    assert(nfConnector);

    // The immutable configuration is owned by the blocks Network.framework invokes during the handshake; the
    // results are per connection: the configuration error, and the flag the verification of this connection sets.
    auto configuration =
        make_shared<const IceInternal::NetworkFrameworkTLS::ClientConfiguration>(*clientAuthenticationOptions, _host);
    auto localVerifyRejected = make_shared<atomic<bool>>(false);
    __block exception_ptr error;

    auto parameters = IceInternal::NetworkRef<nw_parameters_t>::adopt(nw_parameters_create_secure_tcp(
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

    auto transceiver =
        nfConnector->connect(IceInternal::NetworkFrameworkConnector::Protocol::TLS, std::move(parameters));
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
