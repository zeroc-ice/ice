//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SSLConnectorI.h"

#include "SSLInstance.h"
#include <utility>

#include "../NetworkProxy.h"
#include "../StreamSocket.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "SSLEndpointI.h"
#include "SSLEngine.h"
#include "SSLUtil.h"

#if defined(ICE_USE_SCHANNEL)
#    include "SchannelEngine.h"
#    include "SchannelTransceiverI.h"
using namespace Ice::SSL::Schannel;
#elif defined(ICE_USE_SECURE_TRANSPORT)
#    include "SecureTransportEngine.h"
#    include "SecureTransportTransceiverI.h"
using namespace Ice::SSL::SecureTransport;
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
    return make_shared<TransceiverI>(_instance, _delegate->connect(), _host, *clientAuthenticationOptions);
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
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
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
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
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

Ice::SSL::ConnectorI::~ConnectorI() {}
