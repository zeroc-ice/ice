//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SSLConnectorI.h"
#include "SSLInstance.h"

#include "../NetworkProxy.h"
#include "../StreamSocket.h"
#include "Ice/Communicator.h"
#include "Ice/LocalException.h"
#include "Ice/LoggerUtil.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "SSLEndpointI.h"
#include "SSLEngine.h"
#include "SSLUtil.h"

#if defined(_WIN32)
#    include "SChannelEngine.h"
#    include "SChannelTransceiverI.h"
#elif defined(__APPLE__)
#    include "SecureTransportEngine.h"
#    include "SecureTransportTransceiverI.h"
#else
#    include "OpenSSLEngine.h"
#    include "OpenSSLTransceiverI.h"
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
#if defined(_WIN32)
    return make_shared<SChannel::TransceiverI>(_instance, _delegate->connect(), _host, *clientAuthenticationOptions);
#elif defined(__APPLE__)
    return make_shared<SecureTransport::TransceiverI>(
        _instance,
        _delegate->connect(),
        _host,
        *clientAuthenticationOptions);
#else
    return make_shared<OpenSSL::TransceiverI>(_instance, _delegate->connect(), _host, *clientAuthenticationOptions);
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

Ice::SSL::ConnectorI::ConnectorI(const InstancePtr& instance, const IceInternal::ConnectorPtr& del, const string& h)
    : _instance(instance),
      _delegate(del),
      _host(h)
{
}

Ice::SSL::ConnectorI::~ConnectorI() {}
